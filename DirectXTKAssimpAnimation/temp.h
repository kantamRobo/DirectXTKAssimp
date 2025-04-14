#pragma once
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
class Temp {
public:
    Assimp::Importer importer;
    //TODO
    //頂点バッファとインデックスバッファの生成
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    struct CB_Bones {
        DirectX::XMMATRIX boneMatrices[100];
    };

    struct CB_PerObject {
        DirectX::XMMATRIX worldViewProj;
    };
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

    std::vector<DirectX::XMMATRIX> boneTransforms;
    Microsoft::WRL::ComPtr<ID3D11Buffer> perObjectCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> boneCB;
    void Init(ID3D11Device* device);
    void LoadMesh(aiMesh* mesh, std::vector<Vertex>& outVertices, std::vector<UINT>& outIndices, std::map<std::string, int>& boneMapping);
    void Render(ID3D11DeviceContext* context);
    Microsoft::WRL::ComPtr <ID3D11Buffer> CreateIndexBuffer(ID3D11Device* device, const std::vector<UINT>& indices);
    template<typename TVertex>
    Microsoft::WRL::ComPtr <ID3D11Buffer> CreateVertexBuffer(ID3D11Device* device, const std::vector<TVertex>& vertices);
    template<typename TCBuffer>
    Microsoft::WRL::ComPtr <ID3D11Buffer> CreateConstantBuffer(ID3D11Device* device);
};