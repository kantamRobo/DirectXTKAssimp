#include "pch.h"

#pragma once
#include <memory>
#include <CommonStates.h>
#include <DirectXMath.h>
#include <BufferHelpers.h>
#include <DeviceResources.h>
#include <vector>
#include <d3dcompiler.h>
#include <VertexTypes.h>


struct SceneConstantBuffer
{
    DirectX::XMFLOAT4 offset = { 0,0,0,1 };
    float padding[60]; // Padding so the constant buffer is 256-byte aligned.
};

class DirectXTKHelloConstTexture
{
public:
    DirectXTKHelloConstTexture() {};
    DirectXTKHelloConstTexture(DX::DeviceResources* DR, UINT width, UINT height, std::wstring name);

    void OnUpdate(DX::DeviceResources* DR);

    void Draw(const DX::DeviceResources* DR);

    HRESULT CreateShaders(const DX::DeviceResources* deviceResources);

    HRESULT CreateBuffers(DX::DeviceResources* DR, int width, int height);
    HRESULT CreateTexture(DX::DeviceResources* DR, const wchar_t* path);
   
    std::vector<DirectX::VertexPositionColorTexture> vertices;
    std::vector<UINT> indices;
private:
    static const UINT FrameCount = 2;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    std::unique_ptr<DirectX::CommonStates> state;
    //シェーダーの作成
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_modelInputLayout = nullptr;// 入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerstate;
    void OnUpdate();
    SceneConstantBuffer sceneCB;
    DirectX::ConstantBuffer<SceneConstantBuffer>m_constantBufferData;

};


