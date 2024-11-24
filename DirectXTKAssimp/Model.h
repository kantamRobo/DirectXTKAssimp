#pragma once
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "Vertex.h"
namespace education {
	class Model 
	{
	public:

		~Model() {};
		bool LoadModel(const char* path);
		std::vector<Vertex> GenerateVertices();
		void Draw();
		Model(const char* path);

		const aiScene* m_scene;
		Assimp::Importer m_importer;


		std::vector<Vertex> vertices;
		std::vector<unsigned short> indices;

		DirectX::XMMATRIX modelmat;
	};

}

// Z�o�b�t�@��L����
// Z�o�b�t�@�ւ̏������݂�L����
// �o�b�N�J�����O�𖳌���