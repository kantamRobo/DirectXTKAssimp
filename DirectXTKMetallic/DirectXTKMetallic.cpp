#include "pch.h"
#include "DirectXTKMetallic.h"
#include <DDSTextureLoader.h>


void DirectXTKMetallic::InitializeMaterialCB(const DX::DeviceResources* DR) {
    // キューブマップ (.dds) をロード
       // ※TextureCubeとして作成されます
    DX::ThrowIfFailed(
        DirectX::CreateDDSTextureFromFileEx(
            device,
            resourceUpload,
            L"E:\\repos\\DirectXTK12Sphere\\DirectXTK12MetalicReflection\\earth-cubemap.dds",
            0,
            D3D12_RESOURCE_FLAG_NONE,
            DirectX::DDS_LOADER_DEFAULT,
            m_envMapTexture.ReleaseAndGetAddressOf())
    );


    ComPtr<ID3D11ShaderResourceView> srv;
    HRESULT hr = CreateDDSTextureFromFile(d3dDevice.Get(), L"SEAFLOOR.DDS",
        nullptr, srv.GetAddressOf());
    DX::ThrowIfFailed(hr);


    // SRV (Shader Resource View) をヒープに作成
    DirectX::CreateShaderResourceView(
        device,
        m_envMapTexture.Get(),
        m_resourceDescriptors->GetFirstCpuHandle(),
        true // isCubeMap = true (TextureCubeとして扱うために重要)
    );
    // ---------------------------------------------------------
    // ★ここを追加！: サンプラーの中身を作成する
    // ---------------------------------------------------------
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // 線形補間
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 繰り返し
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

    // サンプラーヒープの先頭ハンドルに書き込む
    device->CreateSampler(&samplerDesc, m_samplerHeap->GetFirstCpuHandle());
}



