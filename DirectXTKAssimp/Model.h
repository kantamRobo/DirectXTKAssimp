#pragma once
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <VertexTypes.h>
#include <DeviceResources.h>
namespace education {
	class Model 
	{
	public:

		~Model() {};
		Model(DX::DeviceResources* deviceresources, const char* path);
		bool LoadModel(const char* path);
		std::vector<DirectX::VertexPositionNormalColorTexture> GenerateVertices();
		HRESULT CreateBuffer(DX::DeviceResources* deviceResources);
		HRESULT CreateShaders(const DX::DeviceResources* deviceResources);
		void CreateBuffers(const DX::DeviceResources* deviceResources);
		HRESULT craetepipelineState(const DX::DeviceResources* deviceResources);
		void Draw(const DX::DeviceResources* DR );
		

		const aiScene* m_scene;
		Assimp::Importer m_importer;

		//�o�b�t�@
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;


		std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
		std::vector<unsigned short> indices;

		DirectX::XMMATRIX modelmat;
		//�V�F�[�_�[�̍쐬
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_modelInputLayout = nullptr;// ���̓��C�A�E�g
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	};

}

// Z�o�b�t�@��L����
// Z�o�b�t�@�ւ̏������݂�L����
// �o�b�N�J�����O�𖳌���