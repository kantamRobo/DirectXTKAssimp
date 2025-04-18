#include "pch.h"

#pragma once
#include <DirectXMath.h>
#include <BufferHelpers.h>
#include <DeviceResources.h>
#include <vector>

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

struct SceneConstantBuffer
{
    DirectX::XMFLOAT4 offset;
    float padding[60]; // Padding so the constant buffer is 256-byte aligned.
};

class DirectXTKHelloConst
{
public:
    DirectXTKHelloConst(UINT width, UINT height, std::wstring name);

    void OnUpdate(DX::DeviceResources* DR);

    HRESULT CreateBuffers(DX::DeviceResources* DR, int width, int height);

    std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
    std::vector<unsigned short> indices;
private:
    static const UINT FrameCount = 2;

    Microsoft::WRL::ComPtr<ID3D11Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Resource> m_indexBuffer;

    void OnUpdate();
    SceneConstantBuffer sceneCB;
    DirectX::ConstantBuffer<SceneConstantBuffer>m_constantBufferData;
   
};


