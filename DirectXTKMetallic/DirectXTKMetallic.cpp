#include "pch.h"
#include "DirectXTKMetallic.h"



void DirectXTKMetallic::InitializeMaterialCB(const DX::DeviceResources* DR) {
  
	auto d3dDevice = DR->GetD3DDevice();
    
    // キューブマップ (.dds) をロード
       // ※TextureCubeとして作成されます
    


  
    HRESULT hr = DirectX::CreateDDSTextureFromFile(d3dDevice, L"E:\\repos\\DirectXTK12Sphere\\DirectXTK12MetalicReflection\\earth-cubemap.dds",
        nullptr, m_srv.GetAddressOf());
    DX::ThrowIfFailed(hr);


   
   
}





HRESULT DirectXTKMetallic::CreateShaders(const DX::DeviceResources* deviceResources)
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
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA, 0}
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




    return hr;
}