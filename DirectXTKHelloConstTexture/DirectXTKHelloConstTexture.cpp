#include "pch.h"
#include <WICTextureLoader.h>
#include "VertexTypes.h"

#include "DirectXTKHelloConstTexture.h"

using namespace DirectX;
using namespace Microsoft::WRL;




DirectXTKHelloConstTexture::DirectXTKHelloConstTexture(DX::DeviceResources* DR,UINT width, UINT height, std::wstring name)
{
    CreateBuffers(DR, width, height);
    CreateTexture(DR, name.c_str());
    CreateShaders(DR);

}

// Update frame-based values.
void DirectXTKHelloConstTexture::OnUpdate(DX::DeviceResources* DR)
{
    const float translationSpeed = 0.005f;
    const float offsetBounds = 1.25f;


    sceneCB.offset.x += translationSpeed;

    if (sceneCB.offset.x > offsetBounds)
    {
        sceneCB.offset.x = -offsetBounds;
    }
    m_constantBufferData.SetData(DR->GetD3DDeviceContext(), sceneCB);
    auto buffer = m_constantBufferData.GetBuffer();
    DR->GetD3DDeviceContext()->PSSetConstantBuffers(0, 1, &buffer);
    DR->GetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &buffer); // ← これを追加

}



void DirectXTKHelloConstTexture::Draw(const DX::DeviceResources* DR) {
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

    // 頂点バッファの設定
    UINT stride = sizeof(DirectX::VertexPositionColorTexture);
    UINT offset = 0;
    auto vertexBuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    // プリミティブトポロジー設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto buffer = m_constantBufferData.GetBuffer();
    context->VSSetConstantBuffers(0, 1, &buffer);
    context->PSSetConstantBuffers(0, 1, &buffer);

    // シェーダー設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    const auto SRV = srv.GetAddressOf();
    auto samplerState = state->LinearWrap();
    context->PSSetSamplers(0, 1,&samplerState);
    context->PSSetShaderResources(0, 1,SRV);
    context->RSSetState(m_rasterizerState.Get());
    
    // 描画コール   
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}


//シェーダー

HRESULT DirectXTKHelloConstTexture::CreateShaders(const DX::DeviceResources* deviceResources)
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
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                           D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


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

    state = std::make_unique<CommonStates>(device);
    

    return hr;
}

HRESULT DirectXTKHelloConstTexture::CreateBuffers(DX::DeviceResources* DR, int width, int height)
{

    vertices = {
     {DirectX::XMFLOAT3{ -0.5f, -0.5f, 0.0f },DirectX::XMFLOAT4 { 1.0f, 0.0f, 0.0f, 1.0f }, DirectX::XMFLOAT2{ 0.0f, 1.0f } }, // 赤
     {DirectX::XMFLOAT3{  0.5f, -0.5f, 0.0f }, DirectX::XMFLOAT4 { 0.0f, 1.0f, 0.0f, 1.0f },  DirectX::XMFLOAT2{ 1.0f, 1.0f }}, // 緑
        {DirectX::XMFLOAT3 {  0.0f,  0.5f, 0.0f }, DirectX::XMFLOAT4 { 0.0f, 0.0f, 1.0f, 1.0f },  DirectX::XMFLOAT2{ 0.5f, 0.0f }  }  // 青
    };

    indices = { 0, 1, 2 };



    auto device = DR->GetD3DDevice();

    // Vertex Buffer Description
    auto vertexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(DirectX::VertexPositionColorTexture) * vertices.size(), // Total size
        D3D11_BIND_VERTEX_BUFFER,                                           // Bind as vertex buffer
        D3D11_USAGE_DYNAMIC,                                                // Dynamic usage
        D3D11_CPU_ACCESS_WRITE                                              // Allow CPU write access
    );

    
    DX::ThrowIfFailed(
        (DirectX::CreateStaticBuffer(device,
            vertices.data(),
            vertices.size(),
            sizeof(VertexPositionColorTexture),
            D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer.GetAddressOf())));


 

   

    

    DX::ThrowIfFailed(
        CreateStaticBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, m_indexBuffer.GetAddressOf())
    );
    ;

   

    // Create Constant Buffer
    m_constantBufferData.Create(device);





    m_constantBufferData.SetData(DR->GetD3DDeviceContext(), sceneCB);
    return S_OK;
}

HRESULT DirectXTKHelloConstTexture::CreateTexture(DX::DeviceResources* DR,const wchar_t* path)
{
    auto device = DR->GetD3DDevice();
   
    HRESULT hr = DirectX::CreateWICTextureFromFile(device, L"C:\\Users\\hatte\\source\\repos\\DirectXTKAssimp\\DirectXTKHelloConstTexture\\Test.png",
        nullptr, srv.GetAddressOf());
    DX::ThrowIfFailed(hr);
    return S_OK;
}
