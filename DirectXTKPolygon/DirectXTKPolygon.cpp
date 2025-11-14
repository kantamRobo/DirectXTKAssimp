#include "pch.h"
#include "DirectXTKPolygon.h"


#include "VertexTypes.h"

#include <BufferHelpers.h>






DirectXTKPolygon::DirectXTKPolygon(DX::DeviceResources* DR)
{
	CreateBuffers(DR, 800, 600);
    CreateShaders(DR);
	
}

void DirectXTKPolygon::Draw(const DX::DeviceResources* DR) {
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
    UINT stride = sizeof(DirectX::VertexPositionColor);
    UINT offset = 0;
    auto vertexBuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    // プリミティブトポロジー設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  

    // シェーダー設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    //context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
    auto RS = state->CullCounterClockwise();

    context->RSSetState(RS);

    //context->PSSetShaderResources(0, 1, m_modelsrv.GetAddressOf());
    // 描画コール   
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}


//シェーダー

HRESULT DirectXTKPolygon::CreateShaders(const DX::DeviceResources* deviceResources)
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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT numElements = ARRAYSIZE(layout);

    hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_modelInputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }
	state = std::make_unique<DirectX::CommonStates>(device);



    return hr;
}

HRESULT DirectXTKPolygon::CreateBuffers(DX::DeviceResources* DR, int width, int height)
{

    vertices = {
     { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // 赤
     { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // 緑
     { DirectX::XMFLOAT3(0.0f,  0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }  // 青
    };

    indices = { 2,1,0 };



    auto device = DR->GetD3DDevice();

    DX::ThrowIfFailed(
        (DirectX::CreateStaticBuffer(device,
            vertices.data(),
            vertices.size(),
            sizeof(DirectX::VertexPositionColor),
            D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer.GetAddressOf())));








    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, m_indexBuffer.GetAddressOf())
    );

   





    return S_OK;
}




