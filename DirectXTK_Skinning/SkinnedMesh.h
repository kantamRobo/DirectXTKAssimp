#pragma once
#include "pch.h"
#include <d3d11.h>
#include <DeviceResources.h>
#include <BufferHelpers.h>
#include <CommonStates.h>
#include <memory>
class SkinnedMesh
{
	// 1. 定数バッファ構造体 (D3D11は16バイト境界に揃える必要がある)
	struct BoneMatricesCB
	{
		float BoneTransforms[64][16]; // float4x4 は 16個のfloat (64バイト)
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
	//シェーダーの作成
	HRESULT CreateShaders(const DX::DeviceResources* deviceResources);
	//パイプラインステートの作成
	HRESULT craetepipelineState(const DX::DeviceResources* deviceResources);
	//描画
	void Draw(const DX::DeviceResources* deviceResources);

	DirectX::ConstantBuffer<DirectX::XMMATRIX> m_boneMatrixBuffer;
	//シーン全体のワールド行列・ビュー行列・プロジェクション行列を格納する定数バッファ
	DirectX::ConstantBuffer<DirectX::XMMATRIX> sceneConstantBuffer;
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

