#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <DeviceResources.h>
#include <string>
#include <vector>
#include <unordered_map>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const int MAX_JOINTS = 128;

struct JointConstantBuffer {
    DirectX::XMFLOAT4X4 JointMatrices[MAX_JOINTS];
};

struct SkinnedVertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexCoord;
    uint32_t JointIndices[4] = { 0, 0, 0, 0 };
    DirectX::XMFLOAT4 JointWeights = { 0.0f, 0.0f, 0.0f, 0.0f };

    void AddBoneData(uint32_t boneID, float weight) {
        float* weights = &JointWeights.x;
        for (int i = 0; i < 4; ++i) {
            if (weights[i] == 0.0f) {
                JointIndices[i] = boneID;
                weights[i] = weight;
                return;
            }
        }
    }
};

struct BoneInfo {
    uint32_t id;
    DirectX::XMFLOAT4X4 offsetMatrix; // ボーン空間への逆バインドポーズ行列
};

class SkinnedModel {
public:
    bool LoadFromFile(DX::DeviceResources* deviceResources, const std::string& filePath);
    bool CreateShaderResources(DX::DeviceResources* deviceResources);
    void Render(DX::DeviceResources* deviceResources);

private:
    void ProcessMesh(aiMesh* mesh, const aiScene* scene,
        std::vector<SkinnedVertex>& vertices,
        std::vector<uint32_t>& indices);

    // DirectX 11 リソース
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pJointCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pCameraCBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;

    uint32_t indexCount = 0;
    int jointCount = 0;

    // ボーン管理
    std::unordered_map<std::string, BoneInfo> boneMapping;
    std::vector<DirectX::XMMATRIX> finalJointMatrices;
};