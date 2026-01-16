#include "pch.h"

#pragma once
#include <wrl/client.h>
#include <DirectXMath.h>
#include <BufferHelpers.h>
#include <DeviceResources.h>
#include <vector>
#include <d3dcompiler.h>
#include <VertexTypes.h>
#include <DDSTextureLoader.h>
#include <CommonStates.h>
#include <memory>

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


// HLSLの cbuffer MaterialBuffer に対応
struct MetallicCB
{
    DirectX::XMFLOAT3 CameraPos;
    float Padding1;             // float3の後ろの隙間埋め

    DirectX::XMFLOAT3 AlbedoColor;
    float Roughness;            // ここでちょうど16バイト境界 (12 + 4)

    float F0;
    float Padding2[3];          // 構造体サイズを16の倍数にするためのパディング
};



struct MaterialCB
{
    DirectX::XMFLOAT4 BaseColor;           // RGBA
    DirectX::XMFLOAT3 Emissive; float _pad; // 16B境界合わせ
    DirectX::XMFLOAT4 Params;              // x=Roughness, y=Metallic, z=Opacity, w=Unused
};


class DirectXTKMetallic
{
public:
    DirectXTKMetallic() {};
    DirectXTKMetallic(UINT width, UINT height, std::wstring name);

    void OnUpdate(DX::DeviceResources* DR);

    void Draw(const DX::DeviceResources* DR);

    HRESULT CreateShaders(const DX::DeviceResources* deviceResources);

    HRESULT CreateBuffers(DX::DeviceResources* DR, int width, int height);

    void InitializeMaterialCB(const DX::DeviceResources* DR);

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
   
    SceneCB sceneCB;
    DirectX::ConstantBuffer<MaterialCB> m_materialcb;
    MaterialCB updates{};
    DirectX::ConstantBuffer< MetallicCB> metalicCB;
    DirectX::ConstantBuffer<SceneCB> m_SceneBuffer;
    Microsoft::WRL:: ComPtr<ID3D11ShaderResourceView> m_srv=nullptr;
    


    std::unique_ptr<DirectX::CommonStates> m_states = nullptr;


}; 