#include "pch.h"
#include "TexturedModel.h"

TexturedModel::TexturedModel(DX::DeviceResources* deviceresources, const char* path, int width, int height)
{


}

HRESULT TexturedModel::CreateTexture(ID3D11Device* device)
{

    // WIC �t�@�C���̃��[�h (DirectXTex �g�p)
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromWICFile(L"C:\\Users\\hatte\\source\\repos\\DirectXTKAssimp\\DirectXTKAssimp\\test.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to load texture file.\n");
        return hr;
    }

    // Direct3D �e�N�X�`�����\�[�X�̍쐬
    Microsoft::WRL::ComPtr<ID3D11Resource> texture;
    hr = DirectX::CreateTexture(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &texture);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Direct3D texture.\n");
        return hr;
    }

    // �V�F�[�_�[���\�[�X�r���[�̍쐬
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = image.GetMetadata().format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(image.GetMetadata().mipLevels);

    hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &m_textureSRV);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Shader Resource View.\n");
        return hr;
    }

    // �T���v���[�X�e�[�g�̍쐬
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&samplerDesc, &m_samplerState);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Sampler State.\n");
        return hr;
    }

    return S_OK;

	
}
