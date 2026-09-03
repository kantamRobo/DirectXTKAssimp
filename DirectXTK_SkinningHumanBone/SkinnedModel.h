#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <DirectXHelpers.h>
#include <wrl.h>
#include <DeviceResources.h>
#include <dxgi1_6.h>
// ボーン配列の定数バッファ用構造体 (16バイトアライメントに注意)
const int MAX_JOINTS = 128; // 例: 最大ボーン数を定義
struct JointConstantBuffer {
    DirectX::XMFLOAT4X4 JointMatrices[MAX_JOINTS]; // 例: MAX_JOINTS = 128 や 256
};


struct SkinnedVertex {
    DirectX::XMFLOAT3 Position;     // POSITION
    DirectX::XMFLOAT3 Normal;       // NORMAL
    DirectX::XMFLOAT2 TexCoord;     // TEXCOORD
    uint32_t JointIndices[4];       // BLENDINDICES (uint4)
    DirectX::XMFLOAT4 JointWeights; // BLENDWEIGHT  (float4)
};

class SkinnedModel
{
	bool CreateShaderResources(DX::DeviceResources* deviceResources);

	Microsoft::WRL::ComPtr<ID3D11Buffer> pJointCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pCameraCBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
    void Render(DX::DeviceResources* deviceResources);
    // ボーン配列の定数バッファ
};



