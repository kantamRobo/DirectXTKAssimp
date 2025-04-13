#include <vector>
#include <stdexcept>
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d11.h>
#include <map>
#include <wrl.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include "pch.h"
Assimp::Importer importer;
std::vector<Vertex> vertices;
std::vector<UINT> indices;
// C++側の定数バッファ構造（要：16バイトアライメント）
struct CB_Bones {
    DirectX::XMMATRIX boneMatrices[100];
};
ID3D11Buffer* boneBuffer = nullptr;


// ボーン名 -> index
std::map<std::string, int> boneMapping;

// 各ボーンに関する情報（最終行列とオフセット）
struct BoneInfo {
    DirectX::XMMATRIX offsetMatrix;   // aiBone::mOffsetMatrix
    DirectX::XMMATRIX finalTransform; // 最終行列（GPUに渡す）
};
std::vector<BoneInfo> boneInfos;
std::vector<DirectX::XMMATRIX> boneTransforms;


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
struct CB_PerObject {
    DirectX::XMMATRIX worldViewProj;
};

ID3D11Buffer* vertexBuffer = nullptr;
ID3D11Buffer* perObjectCB = nullptr;
void CreatePerObjectConstantBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(CB_PerObject);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&desc, nullptr, &perObjectCB);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create perObjectCB");
    }
}


// ボーン名 -> index
ID3D11Buffer* boneCB = nullptr;

void CreateBoneConstantBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(CB_Bones); // 100個のXMMATRIX
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&desc, nullptr, &boneCB);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create boneCB constant buffer");
    }
}

void CreateVertexBuffer(ID3D11Device* device, const std::vector<Vertex>& vertices) {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create vertex buffer");
    }
}

ID3D11Buffer* indexBuffer = nullptr;

void CreateIndexBuffer(ID3D11Device* device, const std::vector<UINT>& indices) {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices.data();

    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &indexBuffer);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create index buffer");
    }
}


void SetInputLayout(ID3D11Device* device)
{

    D3D11_INPUT_ELEMENT_DESC layout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex, normal),   D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BONEIDS",  0, DXGI_FORMAT_R32G32B32A32_UINT,  0, offsetof(Vertex, boneIDs),  D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, weights),  D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

}

void LoadMesh(aiMesh* mesh, std::vector<Vertex>& outVertices, std::vector<UINT>& outIndices, std::map<std::string, int>& boneMapping) {
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



void Render(ID3D11DeviceContext* context){
// ボーン行列はアニメーション計算から得た行列を格納
D3D11_MAPPED_SUBRESOURCE mapped;
context->Map(boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
memcpy(mapped.pData, boneTransforms.data(), sizeof(CB_Bones));
context->Unmap(boneBuffer, 0);

UINT stride = sizeof(Vertex);
UINT offset = 0;
context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

context->VSSetConstantBuffers(0, 1, &perObjectCB);
context->VSSetConstantBuffers(1, 1, &boneCB);
context->DrawIndexed(indices.size(), 0, 0);
}






void UpdateBoneConstantBuffer(ID3D11DeviceContext* context) {
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    CB_Bones* cb = reinterpret_cast<CB_Bones*>(mapped.pData);
    for (size_t i = 0; i < boneInfos.size(); ++i) {
        cb->boneMatrices[i] = boneInfos[i].finalTransform; // すでにTranspose済み
    }

    context->Unmap(boneBuffer, 0);
}

