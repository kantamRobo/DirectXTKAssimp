#include "pch.h"

#pragma once
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
struct SceneCB {
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
};

class DirectXTKHelloConstSceneRendering
{
public:
    DirectXTKHelloConstSceneRendering() {};
    DirectXTKHelloConstSceneRendering(UINT width, UINT height, std::wstring name);

    void OnUpdate(DX::DeviceResources* DR);

    void Draw(const DX::DeviceResources* DR);

    HRESULT CreateShaders(const DX::DeviceResources* deviceResources);

    HRESULT CreateBuffers(DX::DeviceResources* DR, int width, int height);

    std::vector<DirectX::VertexPositionColor> vertices;
    std::vector<UINT> indices;
private:
    static const UINT FrameCount = 2;

    Microsoft::WRL::ComPtr<ID3D11Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Resource> m_indexBuffer;

    //シェーダーの作成
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_modelInputLayout = nullptr;// 入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    void OnUpdate();
    SceneConstantBuffer sceneCB;
    DirectX::ConstantBuffer<SceneConstantBuffer>m_constantBufferData;
    DirectX::ConstantBuffer<SceneCB> m_SceneBuffer;
};

