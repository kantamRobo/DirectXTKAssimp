#pragma once
#include "pch.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DDSTextureLoader.h>
#include <vector>


namespace education {

	

	class Model 
	{
	public:

		~Model() {};
		
		Model(DX::DeviceResources* deviceresources, const char* path, int width, int height);
		bool LoadModel(const char* path);
		std::vector<DirectX::VertexPositionNormalTangentColorTextureSkinning> GenerateVertices();

		void GenerateBones();
		
		HRESULT CreateShaders(const DX::DeviceResources* deviceResources);
		HRESULT CreateBuffers(const DX::DeviceResources* deviceResources,int width, int height);
		
		HRESULT CreateTexture( ID3D11Device* device);
		
		
		HRESULT craetepipelineState(const DX::DeviceResources* deviceResources);
		
		void Draw(const DX::DeviceResources* DR);
		
	
		
		
		



		const aiScene* m_scene;
		Assimp::Importer m_importer;

		//バッファ
		Microsoft::WRL::ComPtr<ID3D11Resource> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Resource> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Resource> m_textureBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_modelsrv;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
		std::vector<DirectX::VertexPositionNormalTangentColorTextureSkinning> vertices;
		std::vector<unsigned short> indices;
		DirectX::ConstantBuffer<SceneCB> m_constantBuffer;
		
		//シェーダーの作成
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_modelInputLayout = nullptr;// 入力レイアウト
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
		
	};

}
