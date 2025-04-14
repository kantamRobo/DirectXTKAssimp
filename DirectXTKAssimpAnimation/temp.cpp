
#include "temp.h"

void Temp::Init(ID3D11Device* device){
const aiScene* scene = importer.ReadFile("model.fbx",
    aiProcess_Triangulate | aiProcess_LimitBoneWeights |
    aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs);

if (!scene || !scene->HasMeshes() || !scene->HasAnimations()) {
    throw std::runtime_error("Model loading failed.");
}

// 使用例
boneCB = CreateConstantBuffer<CB_Bones>(device);
perObjectCB = CreateConstantBuffer<CB_PerObject>(device);
}

void Temp::LoadMesh(aiMesh * mesh, std::vector<Vertex>&outVertices, std::vector<UINT>&outIndices, std::map<std::string, int>&boneMapping) {
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

void Temp::Render(ID3D11DeviceContext* context)
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

    context->VSSetConstantBuffers(0, 1,&perObjectCB);
    context->VSSetConstantBuffers(1, 1, &boneCB);
    context->DrawIndexed(indices.size(), 0, 0);
}

template<typename TVertex>
Microsoft::WRL::ComPtr<ID3D11Buffer> Temp::CreateVertexBuffer(ID3D11Device* device, const std::vector<TVertex>& vertices) {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = static_cast<UINT>(sizeof(TVertex) * vertices.size());
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();

    ID3D11Buffer* buffer = nullptr;
    HRESULT hr = device->CreateBuffer(&desc, &initData, &buffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create vertex buffer");
    return buffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Temp::CreateIndexBuffer(ID3D11Device* device, const std::vector<UINT>& indices) {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices.data();

    ID3D11Buffer* buffer = nullptr;
    HRESULT hr = device->CreateBuffer(&desc, &initData, &buffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create index buffer");
    return buffer;
}


template<typename TCBuffer>
Microsoft::WRL::ComPtr<ID3D11Buffer> Temp::CreateConstantBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(TCBuffer);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* buffer = nullptr;
    HRESULT hr = device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create constant buffer");
    return buffer;
}
