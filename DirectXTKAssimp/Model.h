#pragma once
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <VertexTypes.h>
namespace education {
	class Model 
	{
	public:

		~Model() {};
		bool LoadModel(const char* path);
		std::vector<DirectX::VertexPositionNormalColorTexture> GenerateVertices();
		void Draw();
		Model(const char* path);

		const aiScene* m_scene;
		Assimp::Importer m_importer;


		std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
		std::vector<unsigned short> indices;

		DirectX::XMMATRIX modelmat;
	};

}

// Zバッファを有効化
// Zバッファへの書き込みを有効化
// バックカリングを無効化