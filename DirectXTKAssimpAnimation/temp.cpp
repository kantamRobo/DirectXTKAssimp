#include "pch.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <vector>
#include <map>
#include <string>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
Assimp::Importer importer;
//TODO
//頂点バッファとインデックスバッファの生成
Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;


void Init(){
const aiScene* scene = importer.ReadFile("model.fbx",
    aiProcess_Triangulate | aiProcess_LimitBoneWeights |
    aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs);

if (!scene || !scene->HasMeshes() || !scene->HasAnimations()) {
    throw std::runtime_error("Model loading failed.");
}

}

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
    UINT boneIDs[4] = {};
    float weights[4] = {};
};
std::vector<Vertex> vertices;
std::vector<UINT> indices;
std::map<std::string, int> boneMapping;
void LoadMesh(aiMesh * mesh, std::vector<Vertex>&outVertices, std::vector<UINT>&outIndices, std::map<std::string, int>&boneMapping) {
    outVertices.resize(mesh->mNumVertices);

    // 頂点データ格納
    for (UINT i = 0; i < mesh->mNumVertices; ++i) {
        Vertex& v = outVertices[i];
        v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        if (mesh->HasTextureCoords(0)) {
            v.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
    }

    // ボーンの対応
    for (UINT boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        aiBone* bone = mesh->mBones[boneIndex];
        std::string name(bone->mName.data);
        int index = 0;

        if (boneMapping.count(name) == 0) {
            index = static_cast<int>(boneMapping.size());
            boneMapping[name] = index;
        }
        else {
            index = boneMapping[name];
        }

        // 各頂点にボーン重みを割り当て
        for (UINT w = 0; w < bone->mNumWeights; ++w) {
            UINT vertexId = bone->mWeights[w].mVertexId;
            float weight = bone->mWeights[w].mWeight;
            for (int j = 0; j < 4; ++j) {
                if (outVertices[vertexId].weights[j] == 0.0f) {
                    outVertices[vertexId].boneIDs[j] = index;
                    outVertices[vertexId].weights[j] = weight;
                    break;
                }
            }
        }
    }

    // インデックス
    for (UINT i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; ++j) {
            outIndices.push_back(face.mIndices[j]);
        }
    }
}



Microsoft::WRL::ComPtr < ID3D11Buffer> boneBuffer;
//TODO boneTransformsperObjectCBboneCBの作成

struct CB_Bones {
    DirectX::XMMATRIX boneMatrices[100];
};
std::vector<DirectX::XMMATRIX> boneTransforms;
Microsoft::WRL::ComPtr<ID3D11Buffer> perObjectCB;
Microsoft::WRL::ComPtr<ID3D11Buffer> boneCB;
void Render(ID3D11DeviceContext* context)
{


    // ボーン行列はアニメーション計算から得た行列を格納
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(boneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, boneTransforms.data(), sizeof(CB_Bones));
    context->Unmap(boneBuffer.Get(), 0);


    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->VSSetConstantBuffers(0, 1, &perObjectCB);
    context->VSSetConstantBuffers(1, 1, &boneCB);
    context->DrawIndexed(indices.size(), 0, 0);
}