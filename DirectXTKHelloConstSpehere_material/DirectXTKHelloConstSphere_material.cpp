#include "pch.h"
#include "DirectXTKHelloConstSphere_material.h"
using namespace DirectX;

HRESULT DirectXTKHelloConstSphere_material::CreateBuffers(DX::DeviceResources* DR, int width, int height)
{

    constexpr int slices = 32;  // 経度方向の分割数
    constexpr int stacks = 32;  // 緯度方向の分割数
    constexpr float radius = 1.0f;

    vertices.clear();
    for (int i = 0; i <= stacks; ++i)
    {
        float phi = XM_PI * i / stacks;  // 0〜π
        float y = cosf(phi);
        float r = sinf(phi);

        for (int j = 0; j <= slices; ++j)
        {
            float theta = XM_2PI * j / slices;  // 0〜2π

            float x = r * cosf(theta);
            float z = r * sinf(theta);

            DirectX::VertexPositionNormalTexture vertex = {};
            vertex.position = { x * radius, y * radius, z * radius };
            vertex.normal = { x, y, z }; // 法線は球面上の位置ベクトルと一致
            vertices.push_back(vertex);
        }
    }


    indices.clear();
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            // 1枚目の三角形
            indices.push_back(static_cast<uint16_t>(first));
            indices.push_back(static_cast<uint16_t>(second));
            indices.push_back(static_cast<uint16_t>(first + 1));

            // 2枚目の三角形
            indices.push_back(static_cast<uint16_t>(second));
            indices.push_back(static_cast<uint16_t>(second + 1));
            indices.push_back(static_cast<uint16_t>(first + 1));
        }
    }


    auto device = DR->GetD3DDevice();
    float aspect = float(width) / float(height);

    // Vertex Buffer Description
    auto vertexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(DirectX::VertexPositionNormalTexture) * vertices.size(), // Total size
        D3D11_BIND_VERTEX_BUFFER,                                           // Bind as vertex buffer
        D3D11_USAGE_DYNAMIC,                                                // Dynamic usage
        D3D11_CPU_ACCESS_WRITE                                              // Allow CPU write access
    );


    // Initial data for Vertex Buffer
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();

    // Create Vertex Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBufferTemp;
    HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBufferTemp);
    if (FAILED(hr))
    {
        return hr;
    }

    // Store as ID3D11Resource
    m_vertexBuffer = vertexBufferTemp;

    // Index Buffer Description
    auto indexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(UINT) * indices.size(), // Total size
        D3D11_BIND_INDEX_BUFFER,       // Bind as index buffer
        D3D11_USAGE_DYNAMIC,           // Dynamic usage
        D3D11_CPU_ACCESS_WRITE         // Allow CPU write access
    );

    // Initial data for Index Buffer
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    // Create Index Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBufferTemp;
    hr = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBufferTemp);
    if (FAILED(hr))
    {
        return hr;
    }

    // Store as ID3D11Resource
    m_indexBuffer = indexBufferTemp;

    // 初期値
    MaterialCB mat{};
    mat.BaseColor = DirectX::XMFLOAT4(1.0f, 0.85f, 0.7f, 1.0f);  // ほんのり暖色
    mat.Emissive = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    mat.Params = DirectX::XMFLOAT4(0.5f, 0.0f, 1.0f, 0.0f);   // Roughness=0.5, Metallic=0, Opacity=1

    m_materialcb.Create(device);
    // Store as ID3D11Resource
    m_indexBuffer = indexBufferTemp;
    // Update Constant Buffer  // Set up Matrices
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -10.0f, 0.0f);
    DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

    float fov = DirectX::XMConvertToRadians(45.0f);
    aspect = static_cast<float>(width) / static_cast<float>(height);
    float nearZ = 0.1f;
    float farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);


    // Update Constant Buffer
    SceneCB cb = {};
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));
    m_SceneBuffer.Create(device);
    m_SceneBuffer.SetData(DR->GetD3DDeviceContext(), cb);

  

    m_materialcb.SetData(DR->GetD3DDeviceContext(), mat);

    return S_OK;
}



// Update frame-based values.
void DirectXTKHelloConstSphere_material::OnUpdate(DX::DeviceResources* DR)
{
    const float translationSpeed = 0.005f;
    const float offsetBounds = 1.25f;



    auto buffer = m_SceneBuffer.GetBuffer();
    DR->GetD3DDeviceContext()->PSSetConstantBuffers(0, 1, &buffer);
    DR->GetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &buffer); // ← これを追加

}



void DirectXTKHelloConstSphere_material::Draw(const DX::DeviceResources* DR) {
    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("Vertex or index buffer is empty.\n");
        return;
    }

    auto context = DR->GetD3DDeviceContext();

    // Input Layout 設定
    context->IASetInputLayout(m_modelInputLayout.Get());

    // インデックスバッファの設定
    auto indexBuffer = static_cast<ID3D11Buffer*>(m_indexBuffer.Get());
    // Draw() の中
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 頂点バッファの設定// 修正前: UINT stride = sizeof(DirectX::VertexPositionColor);
    UINT stride = sizeof(DirectX::VertexPositionNormalTexture);  // ← これが正しい
    UINT offset = 0;
    ID3D11Buffer* vbuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vbuffer, &stride, &offset);
    ;
    auto vertexBuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    // プリミティブトポロジー設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->PSSetShaderResources(0, 1, g_Texture.GetAddressOf());
    auto buffer = m_SceneBuffer.GetBuffer();
    context->VSSetConstantBuffers(0, 1, &buffer);
    context->PSSetConstantBuffers(0, 1, &buffer);
    auto buffermat = m_materialcb.GetBuffer();
    context->VSSetConstantBuffers(1, 1, &buffermat);
    context->PSSetConstantBuffers(1, 1, &buffermat);
  

    // シェーダー設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    //context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
    context->OMSetBlendState(states->Opaque(), Colors::Black, 0xFFFFFFFF);

    context->RSSetState(states->CullCounterClockwise());

    auto samplerState = states->LinearWrap();
    context->PSSetSamplers(0, 1, &samplerState);
    //context->PSSetShaderResources(0, 1, m_modelsrv.GetAddressOf());
    // 描画コール   
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}


//シェーダー

HRESULT DirectXTKHelloConstSphere_material::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //パイプラインステートの作成
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // 頂点シェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
    auto hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    // 頂点シェーダーの作成
    hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }


    // ピクセルシェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    //ピクセルシェーダーの作成
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                              D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    // CreateInputLayout(layout, _countof(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), ...)


    UINT numElements = ARRAYSIZE(layout);

    hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_modelInputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }
    if (!m_rasterizerState)
    {
        D3D11_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_NONE; // ← バックフェイスカリングを一時的にオフに
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.DepthClipEnable = true;

        device->CreateRasterizerState(&rasterDesc, &m_rasterizerState);
    }
    states = std::make_unique<CommonStates>(device);



    return hr;
}
