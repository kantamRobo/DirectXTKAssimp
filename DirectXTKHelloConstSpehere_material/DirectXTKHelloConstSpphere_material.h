#include "pch.h"

#pragma once
#include <DirectXMath.h>
#include <BufferHelpers.h>
#include <DeviceResources.h>
#include <vector>
#include <d3dcompiler.h>
#include <VertexTypes.h>
#include <BufferHelpers.h>

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



struct MaterialCB
{
    DirectX::XMFLOAT4 BaseColor;           // RGBA
    DirectX::XMFLOAT3 Emissive; float _pad; // 16B境界合わせ
    DirectX::XMFLOAT4 Params;              // x=Roughness, y=Metallic, z=Opacity, w=Unused
};


class DirectXTKHelloConstSpphere_material
{
public:
    DirectXTKHelloConstSpphere_material() {};
    DirectXTKHelloConstSpphere_material(UINT width, UINT height, std::wstring name);

    void OnUpdate(DX::DeviceResources* DR);

    void Draw(const DX::DeviceResources* DR);

    HRESULT CreateShaders(const DX::DeviceResources* deviceResources);

    HRESULT CreateBuffers(DX::DeviceResources* DR, int width, int height);

    std::vector<DirectX::VertexPositionNormalTexture> vertices;
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
    SceneCB sceneCB;
    DirectX::ConstantBuffer<MaterialCB> m_materialcb;
    MaterialCB updates{};

    DirectX::ConstantBuffer<SceneCB> m_SceneBuffer;

};
