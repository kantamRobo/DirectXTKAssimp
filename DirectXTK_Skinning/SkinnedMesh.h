#pragma once
#include "pch.h"
#include <d3d11.h>
#include <DeviceResources.h>
#include <BufferHelpers.h>
#include <CommonStates.h>
#include <memory>
#include <vector>
class SkinnedMesh
{
	// 1. 定数バッファ構造体 (D3D11は16バイト境界に揃える必要がある)
	struct BoneMatricesCB
	{
		// 64 個の float4x4 -> 64 * 16 * 4 = 4096 bytes
		float BoneTransforms[64][16];
	};

	// シーン用定数バッファ構造体 (World, View, Projection)
	struct SceneCB
	{
		float World[16];
		float View[16];
		float Projection[16];
	};

	// 1. 頂点構造体
	struct VS_INPUT
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMUINT4 boneIds;
		DirectX::XMFLOAT4 weights;
	};

	 UINT vertexCount = 0;
	UINT vertexStride = sizeof(VS_INPUT);
public:
	//頂点バッファ・インデックスバッファの作成
	HRESULT CreateBuffers(const DX::DeviceResources* deviceResources, int width, int height);
	void CreatePipelineStates(const DX::DeviceResources* deviceResources, SkinnedMesh* mesh);
	//シェーダーの作成
	HRESULT CreateShaders(const DX::DeviceResources* deviceResources);
	
	//描画
	void Draw(const DX::DeviceResources* deviceResources);

	// 修正: ボーンは 64 行列分の定数バッファ
	DirectX::ConstantBuffer<BoneMatricesCB> m_boneMatrixBuffer;
	BoneMatricesCB boneMatricesData; // CPU側のデータ保持用
	// シーン定数バッファ (W,V,P)
	DirectX::ConstantBuffer<SceneCB> sceneConstantBuffer;
	SceneCB sceneData; // CPU側のデータ保持用

	// 更新用ヘルパー
	HRESULT UpdateBoneTransforms(const DX::DeviceResources* deviceResources, const std::vector<DirectX::XMFLOAT4X4>& boneTransforms);
	HRESULT UpdateSceneMatrices(const DX::DeviceResources* deviceResources, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj);

	//動的処理化された頂点バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	//インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	//ラスタライザーステート
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	//CommonState
	std::unique_ptr<DirectX::CommonStates> m_commonStates;
	//入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	//頂点シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	//ピクセルシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	//頂点シェーダーブロブ
	Microsoft::WRL::ComPtr<ID3DBlob> m_pVSBlob;
	//ピクセルシェーダーブロブ
	Microsoft::WRL::ComPtr<ID3DBlob> m_pPSBlob;

};

