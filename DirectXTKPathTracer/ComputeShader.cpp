#include "pch.h"

#include "ComputeShader.h"

#include <d3dcompiler.h>
#include <array>
#include <chrono>

#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

namespace
{
    std::wstring ResolveShaderPath(const wchar_t* fileName)
    {
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);

        std::wstring fullPath(exePath);
        const auto slashPos = fullPath.find_last_of(L"\\/");
        if (slashPos != std::wstring::npos)
        {
            fullPath = fullPath.substr(0, slashPos + 1);
            fullPath += fileName;
            if (GetFileAttributesW(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES)
            {
                return fullPath;
            }
        }

        return std::wstring(fileName);
    }

    template<typename T>
    std::vector<T> ReserveAndCopy(size_t count)
    {
        std::vector<T> out;
        out.reserve(count);
        return out;
    }
}

void ComputeShader::Initialize(DX::DeviceResources* deviceResources, uint32_t width, uint32_t height)
{
    auto* device = deviceResources->GetD3DDevice();

    m_width = std::max(1u, width);
    m_height = std::max(1u, height);
    m_sampleCount = 0;
    m_randomSeed = 1;
    m_resetAccumulation = true;

    BuildScene();
    BuildLinearBVH();

    CreateShaders(device);
    CreateSceneBuffers(device);
    CreateRenderResources(device, m_width, m_height);

    if (!m_pathTracerCB)
    {
        CD3D11_BUFFER_DESC cbDesc(
            sizeof(pt::PathTracerCB),
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC,
            D3D11_CPU_ACCESS_WRITE);
        DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_pathTracerCB.ReleaseAndGetAddressOf()));
    }

    if (!m_screenVB)
    {
        static const std::array<ScreenVertex, 4> kQuad = {
            ScreenVertex{ { -1.0f, -1.0f }, { 0.0f, 1.0f } },
            ScreenVertex{ { -1.0f,  1.0f }, { 0.0f, 0.0f } },
            ScreenVertex{ {  1.0f, -1.0f }, { 1.0f, 1.0f } },
            ScreenVertex{ {  1.0f,  1.0f }, { 1.0f, 0.0f } },
        };

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = kQuad.data();
        CD3D11_BUFFER_DESC vbDesc(static_cast<UINT>(sizeof(ScreenVertex) * kQuad.size()), D3D11_BIND_VERTEX_BUFFER);
        DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &initData, m_screenVB.ReleaseAndGetAddressOf()));
    }
}

void ComputeShader::OnResize(DX::DeviceResources* deviceResources, uint32_t width, uint32_t height)
{
    auto* device = deviceResources->GetD3DDevice();
    m_width = std::max(1u, width);
    m_height = std::max(1u, height);
    m_resetAccumulation = true;
    m_sampleCount = 0;
    CreateRenderResources(device, m_width, m_height);
}

void ComputeShader::OnDeviceLost()
{
    m_pathTracerCS.Reset();
    m_compositeVS.Reset();
    m_compositePS.Reset();
    m_compositeInputLayout.Reset();
    m_linearSampler.Reset();

    m_pathTracerCB.Reset();
    m_screenVB.Reset();

    m_accumTexture.Reset();
    m_accumUAV.Reset();
    m_outputTexture.Reset();
    m_outputUAV.Reset();
    m_outputSRV.Reset();

    m_sphereBuffer.Reset();
    m_sphereSRV.Reset();
    m_triangleBuffer.Reset();
    m_triangleSRV.Reset();
    m_materialBuffer.Reset();
    m_materialSRV.Reset();
    m_bvhBuffer.Reset();
    m_bvhSRV.Reset();
}

void ComputeShader::SetCamera(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& forward,
    const DirectX::XMFLOAT3& right,
    const DirectX::XMFLOAT3& up,
    float verticalFovRadians,
    bool resetAccumulation)
{
    m_cameraPosition = position;
    m_cameraForward = forward;
    m_cameraRight = right;
    m_cameraUp = up;
    m_verticalFov = verticalFovRadians;

    if (resetAccumulation)
    {
        m_resetAccumulation = true;
        m_sampleCount = 0;
    }
}

void ComputeShader::Render(DX::DeviceResources* deviceResources, float elapsedSeconds)
{
    auto* context = deviceResources->GetD3DDeviceContext();

    UpdateConstantBuffer(context, deviceResources, elapsedSeconds);
    DispatchPathTrace(context);
    CompositeToBackbuffer(context, deviceResources);

    ++m_sampleCount;
    m_randomSeed = 1664525u * m_randomSeed + 1013904223u;
    m_resetAccumulation = false;
}

void ComputeShader::CreateShaders(ID3D11Device* device)
{
    auto csBlob = CompileShaderFromFile(ResolveShaderPath(L"PathTracerCS.hlsl").c_str(), "main", "cs_5_0");
    DX::ThrowIfFailed(device->CreateComputeShader(
        csBlob->GetBufferPointer(),
        csBlob->GetBufferSize(),
        nullptr,
        m_pathTracerCS.ReleaseAndGetAddressOf()));

    auto vsBlob = CompileShaderFromFile(ResolveShaderPath(L"CompositeVS.hlsl").c_str(), "main", "vs_5_0");
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_compositeVS.ReleaseAndGetAddressOf()));

    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(device->CreateInputLayout(
        inputLayout,
        static_cast<UINT>(std::size(inputLayout)),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_compositeInputLayout.ReleaseAndGetAddressOf()));

    auto psBlob = CompileShaderFromFile(ResolveShaderPath(L"CompositePS.hlsl").c_str(), "main", "ps_5_0");
    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_compositePS.ReleaseAndGetAddressOf()));

    CD3D11_SAMPLER_DESC samplerDesc(D3D11_DEFAULT);
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, m_linearSampler.ReleaseAndGetAddressOf()));
}

void ComputeShader::CreateRenderResources(ID3D11Device* device, uint32_t width, uint32_t height)
{
    CD3D11_TEXTURE2D_DESC texDesc(
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        width,
        height,
        1,
        1,
        D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
        D3D11_USAGE_DEFAULT,
        0,
        1,
        0);

    DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, m_accumTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, m_outputTexture.ReleaseAndGetAddressOf()));

    CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2D, texDesc.Format);
    DX::ThrowIfFailed(device->CreateUnorderedAccessView(m_accumTexture.Get(), &uavDesc, m_accumUAV.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateUnorderedAccessView(m_outputTexture.Get(), &uavDesc, m_outputUAV.ReleaseAndGetAddressOf()));

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, texDesc.Format);
    DX::ThrowIfFailed(device->CreateShaderResourceView(m_outputTexture.Get(), &srvDesc, m_outputSRV.ReleaseAndGetAddressOf()));
}

void ComputeShader::CreateSceneBuffers(ID3D11Device* device)
{
    std::vector<pt::MaterialData> gpuMaterials = ReserveAndCopy<pt::MaterialData>(m_materials.size());
    for (const auto& m : m_materials)
    {
        pt::MaterialData data = {};
        data.albedo = { m.albedo.x, m.albedo.y, m.albedo.z };
        data.type = m.type;
        data.fuzz = m.fuzz;
        data.refractiveIndex = m.refractiveIndex;
        data.emissiveStrength = m.emissiveStrength;
        data.emissiveColor = { m.emissiveColor.x, m.emissiveColor.y, m.emissiveColor.z };
        gpuMaterials.push_back(data);
    }

    std::vector<pt::SphereData> gpuSpheres = ReserveAndCopy<pt::SphereData>(m_spheres.size());
    for (const auto& s : m_spheres)
    {
        pt::SphereData data = {};
        data.center = { s.center.x, s.center.y, s.center.z };
        data.radius = s.radius;
        data.materialIndex = s.materialIndex;
        gpuSpheres.push_back(data);
    }

    std::vector<pt::TriangleData> gpuTriangles = ReserveAndCopy<pt::TriangleData>(m_triangles.size());
    for (const auto& t : m_triangles)
    {
        pt::TriangleData data = {};
        data.v0 = { t.v0.x, t.v0.y, t.v0.z };
        data.v1 = { t.v1.x, t.v1.y, t.v1.z };
        data.v2 = { t.v2.x, t.v2.y, t.v2.z };
        data.normal = { t.normal.x, t.normal.y, t.normal.z };
        data.materialIndex = t.materialIndex;
        gpuTriangles.push_back(data);
    }

    std::vector<pt::BVHNodeData> gpuBvh = ReserveAndCopy<pt::BVHNodeData>(m_bvhNodes.size());
    for (const auto& n : m_bvhNodes)
    {
        pt::BVHNodeData data = {};
        data.boundsMin = { n.boundsMin.x, n.boundsMin.y, n.boundsMin.z };
        data.boundsMax = { n.boundsMax.x, n.boundsMax.y, n.boundsMax.z };
        data.leftChild = n.leftChild;
        data.rightChild = n.rightChild;
        data.startIndex = n.startIndex;
        data.primitiveCount = n.primitiveCount;
        data.isLeaf = n.isLeaf;
        gpuBvh.push_back(data);
    }

    m_materialBuffer = CreateStructuredBuffer(device, gpuMaterials, false);
    m_sphereBuffer = CreateStructuredBuffer(device, gpuSpheres, false);
    m_triangleBuffer = CreateStructuredBuffer(device, gpuTriangles, false);
    m_bvhBuffer = CreateStructuredBuffer(device, gpuBvh, false);

    auto createSrv = [device](ID3D11Buffer* buffer, UINT elementCount, ID3D11ShaderResourceView** outSrv)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = elementCount;
        DX::ThrowIfFailed(device->CreateShaderResourceView(buffer, &srvDesc, outSrv));
    };

    createSrv(m_materialBuffer.Get(), static_cast<UINT>(gpuMaterials.size()), m_materialSRV.ReleaseAndGetAddressOf());
    createSrv(m_sphereBuffer.Get(), static_cast<UINT>(gpuSpheres.size()), m_sphereSRV.ReleaseAndGetAddressOf());
    createSrv(m_triangleBuffer.Get(), static_cast<UINT>(gpuTriangles.size()), m_triangleSRV.ReleaseAndGetAddressOf());
    createSrv(m_bvhBuffer.Get(), static_cast<UINT>(gpuBvh.size()), m_bvhSRV.ReleaseAndGetAddressOf());
}

void ComputeShader::BuildScene()
{
    m_materials.clear();
    m_spheres.clear();
    m_triangles.clear();

    m_materials.push_back({ { 0.8f, 0.8f, 0.0f }, 0.0f, 1.0f, 0.0f, { 0.0f, 0.0f, 0.0f }, pt::MaterialLambertian });
    m_materials.push_back({ { 0.7f, 0.3f, 0.3f }, 0.0f, 1.0f, 0.0f, { 0.0f, 0.0f, 0.0f }, pt::MaterialLambertian });
    m_materials.push_back({ { 0.8f, 0.8f, 0.8f }, 0.05f, 1.0f, 0.0f, { 0.0f, 0.0f, 0.0f }, pt::MaterialMetal });
    m_materials.push_back({ { 1.0f, 1.0f, 1.0f }, 0.0f, 1.5f, 0.0f, { 0.0f, 0.0f, 0.0f }, pt::MaterialDielectric });
    m_materials.push_back({ { 0.0f, 0.0f, 0.0f }, 0.0f, 1.0f, 4.0f, { 1.0f, 0.95f, 0.8f }, pt::MaterialEmissive });

    m_spheres.push_back({ { 0.0f, -100.5f, 1.0f }, 100.0f, 0 });
    m_spheres.push_back({ { -1.2f, 0.35f, 2.5f }, 0.85f, 1 });
    m_spheres.push_back({ { 0.5f, 0.35f, 2.1f }, 0.85f, 2 });
    m_spheres.push_back({ { 2.1f, 0.5f, 3.0f }, 1.0f, 3 });
    m_spheres.push_back({ { 0.0f, 4.5f, 1.5f }, 0.8f, 4 });

    const std::vector<DirectX::XMFLOAT3> verts =
    {
        { -1.0f, 0.0f, 4.0f },
        {  1.0f, 0.0f, 4.0f },
        {  0.0f, 1.7f, 4.0f },
        { -0.7f, 0.0f, 5.4f },
        {  0.7f, 0.0f, 5.4f },
        {  0.0f, 1.4f, 5.4f },
    };

    const std::vector<uint32_t> indices =
    {
        0, 1, 2,
        3, 5, 4,
    };

    AppendIndexedTriangles(verts, indices, 1u);
}

void ComputeShader::BuildLinearBVH()
{
    m_bvhNodes.clear();

    if (m_triangles.empty())
    {
        CpuBVHNode node = {};
        node.boundsMin = { -1.0f, -1.0f, -1.0f };
        node.boundsMax = { 1.0f, 1.0f, 1.0f };
        node.leftChild = 0;
        node.rightChild = 0;
        node.startIndex = 0;
        node.primitiveCount = 0;
        node.isLeaf = 1;
        m_bvhNodes.push_back(node);
        return;
    }

    DirectX::XMFLOAT3 minBounds = m_triangles.front().v0;
    DirectX::XMFLOAT3 maxBounds = m_triangles.front().v0;

    auto expand = [&minBounds, &maxBounds](const DirectX::XMFLOAT3& p)
    {
        minBounds.x = std::min(minBounds.x, p.x);
        minBounds.y = std::min(minBounds.y, p.y);
        minBounds.z = std::min(minBounds.z, p.z);
        maxBounds.x = std::max(maxBounds.x, p.x);
        maxBounds.y = std::max(maxBounds.y, p.y);
        maxBounds.z = std::max(maxBounds.z, p.z);
    };

    for (const auto& tri : m_triangles)
    {
        expand(tri.v0);
        expand(tri.v1);
        expand(tri.v2);
    }

    CpuBVHNode node = {};
    node.boundsMin = minBounds;
    node.boundsMax = maxBounds;
    node.leftChild = 0;
    node.rightChild = 0;
    node.startIndex = 0;
    node.primitiveCount = static_cast<uint32_t>(m_triangles.size());
    node.isLeaf = 1;
    m_bvhNodes.push_back(node);
}

void ComputeShader::AppendIndexedTriangles(
    const std::vector<DirectX::XMFLOAT3>& vertices,
    const std::vector<uint32_t>& indices,
    uint32_t materialIndex)
{
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        const auto& v0 = vertices[indices[i + 0]];
        const auto& v1 = vertices[indices[i + 1]];
        const auto& v2 = vertices[indices[i + 2]];

        const auto e1 = DirectX::XMLoadFloat3(&v1) - DirectX::XMLoadFloat3(&v0);
        const auto e2 = DirectX::XMLoadFloat3(&v2) - DirectX::XMLoadFloat3(&v0);
        const auto n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(e1, e2));

        CpuTriangle tri = {};
        tri.v0 = v0;
        tri.v1 = v1;
        tri.v2 = v2;
        DirectX::XMStoreFloat3(&tri.normal, n);
        tri.materialIndex = materialIndex;
        m_triangles.push_back(tri);
    }
}

void ComputeShader::UpdateConstantBuffer(ID3D11DeviceContext* context, DX::DeviceResources* deviceResources, float elapsedSeconds)
{
    pt::PathTracerCB cb = {};
    cb.cameraPosition = { m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z };
    cb.frameIndex = static_cast<float>(m_sampleCount);
    cb.cameraForward = { m_cameraForward.x, m_cameraForward.y, m_cameraForward.z };
    cb.resetAccumulation = m_resetAccumulation ? 1.0f : 0.0f;
    cb.cameraRight = { m_cameraRight.x, m_cameraRight.y, m_cameraRight.z };
    cb.verticalFov = m_verticalFov;
    cb.cameraUp = { m_cameraUp.x, m_cameraUp.y, m_cameraUp.z };
    cb.aspectRatio = static_cast<float>(m_width) / static_cast<float>(std::max(1u, m_height));
    cb.width = m_width;
    cb.height = m_height;
    cb.sphereCount = static_cast<uint32_t>(m_spheres.size());
    cb.triangleCount = static_cast<uint32_t>(m_triangles.size());
    cb.materialCount = static_cast<uint32_t>(m_materials.size());
    cb.maxBounces = m_maxBounces;
    cb.bvhNodeCount = static_cast<uint32_t>(m_bvhNodes.size());
    cb.randomSeed = m_randomSeed;
    cb.elapsedTimeSeconds = elapsedSeconds;
    cb.russianRouletteMin = 0.1f;
    cb.epsilon = 0.0005f;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    DX::ThrowIfFailed(context->Map(m_pathTracerCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(m_pathTracerCB.Get(), 0);

    if (m_resetAccumulation)
    {
        const float clear[4] = { 0.f, 0.f, 0.f, 0.f };
        context->ClearUnorderedAccessViewFloat(m_accumUAV.Get(), clear);
    }

    (void)deviceResources;
}

void ComputeShader::DispatchPathTrace(ID3D11DeviceContext* context)
{
    ID3D11ShaderResourceView* srvs[] =
    {
        m_sphereSRV.Get(),
        m_triangleSRV.Get(),
        m_materialSRV.Get(),
        m_bvhSRV.Get(),
    };

    ID3D11UnorderedAccessView* uavs[] =
    {
        m_accumUAV.Get(),
        m_outputUAV.Get(),
    };

    context->CSSetShader(m_pathTracerCS.Get(), nullptr, 0);
    context->CSSetConstantBuffers(0, 1, m_pathTracerCB.GetAddressOf());
    context->CSSetShaderResources(0, static_cast<UINT>(std::size(srvs)), srvs);
    context->CSSetUnorderedAccessViews(0, static_cast<UINT>(std::size(uavs)), uavs, nullptr);

    const UINT groupX = (m_width + 7u) / 8u;
    const UINT groupY = (m_height + 7u) / 8u;
    context->Dispatch(groupX, groupY, 1u);

    ID3D11ShaderResourceView* nullSrvs[4] = {};
    ID3D11UnorderedAccessView* nullUavs[2] = {};
    context->CSSetShaderResources(0, 4, nullSrvs);
    context->CSSetUnorderedAccessViews(0, 2, nullUavs, nullptr);
    context->CSSetShader(nullptr, nullptr, 0);
}

void ComputeShader::CompositeToBackbuffer(ID3D11DeviceContext* context, DX::DeviceResources* deviceResources)
{
    auto* rtv = deviceResources->GetRenderTargetView();
    auto* dsv = deviceResources->GetDepthStencilView();
    context->OMSetRenderTargets(1, &rtv, dsv);

    const auto viewport = deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    UINT stride = sizeof(ScreenVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_screenVB.GetAddressOf(), &stride, &offset);
    context->IASetInputLayout(m_compositeInputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->VSSetShader(m_compositeVS.Get(), nullptr, 0);
    context->PSSetShader(m_compositePS.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
    context->PSSetShaderResources(0, 1, m_outputSRV.GetAddressOf());

    context->Draw(4, 0);

    ID3D11ShaderResourceView* nullSrv = nullptr;
    context->PSSetShaderResources(0, 1, &nullSrv);
    context->PSSetShader(nullptr, nullptr, 0);
    context->VSSetShader(nullptr, nullptr, 0);
}

template<typename T>
ComPtr<ID3D11Buffer> ComputeShader::CreateStructuredBuffer(
    ID3D11Device* device,
    const std::vector<T>& data,
    bool allowUav,
    bool dynamic)
{
    ComPtr<ID3D11Buffer> buffer;

    const UINT byteWidth = static_cast<UINT>(std::max<size_t>(1, data.size()) * sizeof(T));
    const UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | (allowUav ? D3D11_BIND_UNORDERED_ACCESS : 0);

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = byteWidth;
    desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(T);

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data.empty() ? nullptr : data.data();

    DX::ThrowIfFailed(device->CreateBuffer(&desc, data.empty() ? nullptr : &initData, buffer.ReleaseAndGetAddressOf()));
    return buffer;
}

template<typename T>
void ComputeShader::UploadStructuredBuffer(
    ID3D11DeviceContext* context,
    ID3D11Buffer* buffer,
    const std::vector<T>& data)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    DX::ThrowIfFailed(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    memcpy(mapped.pData, data.data(), data.size() * sizeof(T));
    context->Unmap(buffer, 0);
}

ComPtr<ID3DBlob> ComputeShader::CompileShaderFromFile(
    const wchar_t* path,
    const char* entryPoint,
    const char* profile)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    const auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        profile,
        flags,
        0,
        shaderBlob.GetAddressOf(),
        errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        DX::ThrowIfFailed(hr);
    }

    return shaderBlob;
}
