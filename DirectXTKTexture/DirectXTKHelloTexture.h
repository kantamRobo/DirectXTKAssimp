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




class DirectXTKHelloTexture
{
public:
    DirectXTKHelloTexture() {};
    DirectXTKHelloTexture(DX::DeviceResources* DR, UINT width, UINT height, std::wstring name);

   

    void Draw(DX::DeviceResources* DR);

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
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;



};



