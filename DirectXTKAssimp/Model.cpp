#include "pch.h"
#include "Model.h"
#include "Vertex.h"


#define NOMINMAX

#include <iostream>
#include <cassert>

#include <algorithm>
#include <functional>
#include "Model.h"




bool education::Model::LoadModel(const char* path)
{


    m_scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals);
    if (!m_scene || !m_scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << m_importer.GetErrorString() << std::endl;
        return false;
    }
    vertices = GenerateVertices();
    // �X�P�[���l��ݒ�
    float scaleFactor = 10.0f;


    for (int i = 0; i < vertices.size(); i++)
    {
        //��Z�@vertices[i].position = vertices[i].position,100.0f;
    }

    // ���_����0�łȂ����Ƃ��m�F
    assert(vertices.size() > 0 && "���_�̐����Ɏ��s");

    // �C���f�b�N�X����3�̔{���ł���A���C���f�b�N�X�����_���𒴂��Ȃ����Ƃ��m�F
    assert(indices.size() % 3 == 0 && "�C���f�b�N�X����3�̔{���łȂ�");
    assert(indices.size() <= vertices.size() * 3 && "�C���f�b�N�X�����_���𒴂��Ă��܂�");

    return true;
}

std::vector<DirectX::VertexPositionNormalColorTexture> education::Model::GenerateVertices()
{
    assert(m_scene);



    std::vector< DirectX::VertexPositionNormalColorTexture> outvertices;
    outvertices.clear();

    for (unsigned int i = 0; i < m_scene->mNumMeshes; i++)
    {
        aiMesh* mesh = m_scene->mMeshes[i];



        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            DirectX::VertexPositionNormalColorTexture vertex = {};
            aiVector3D pos = mesh->mVertices[j];


            vertex.position = { pos.x , pos.y , pos.z };
            vertex.normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            if (mesh->mTextureCoords[0])
            {
                vertex.textureCoordinate.x = mesh->mTextureCoords[0][j].x;
                vertex.textureCoordinate.y = mesh->mTextureCoords[0][j].y;
            }
            else
            {
                vertex.textureCoordinate.x = 0.0f;
                vertex.textureCoordinate.y = 0.0f;
            }

            outvertices.push_back(vertex);
        }

        // �C���f�b�N�X�̐ݒ�
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }
    }



    return outvertices;
}

void education::Model::Draw() {
    if (vertices.empty() || indices.empty()) {
        return;
    }

    DrawPolygonIndexed3D(
        vertices.data(),
        static_cast<int>(vertices.size()),
        indices.data(),
        static_cast<int>(indices.size() / 3),
        DX_NONE_GRAPH,
        FALSE
    );
}





education::Model::Model(const char* path)
{
    assert(LoadModel(path), "���[�h�Ɏ��s");
    vertices = GenerateVertices();
}
