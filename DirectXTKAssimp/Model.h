#pragma once
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DxLib.h>
#include <vector>
#include "GameObject.h"
namespace education {
	class Model :public GameObject
	{
	public:

		~Model() {};
		bool LoadModel(const char* path);
		std::vector<VERTEX3D> GenerateVertices();
		void Draw();
		Model(const char* path);

		const aiScene* m_scene;
		Assimp::Importer m_importer;


		std::vector<VERTEX3D> vertices;
		std::vector<unsigned short> indices;

		MATRIX modelmat;
	};

}

// Zバッファを有効化
// Zバッファへの書き込みを有効化
// バックカリングを無効化