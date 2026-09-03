#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <DeviceResources.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <CommonStates.h>]
#include <memory>
#include <BufferHelpers.h>
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
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
   
    //定数バッファはヘルパーを使用
	std::unique_ptr<DirectX::ConstantBuffer<JointConstantBuffer>> m_JointConstantBuffer;
	std::unique_ptr<DirectX::ConstantBuffer<DirectX::XMFLOAT4X4>> m_CameraConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
    //シェーダーブロブ
	Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> m_ErrorBlob;
    //ラスタライズステートのヘルパー
	std::unique_ptr<DirectX::CommonStates> mCommonStates;
    uint32_t indexCount = 0;
    int jointCount = 0;

    // ボーン管理
    std::unordered_map<std::string, BoneInfo> boneMapping;
    std::vector<DirectX::XMMATRIX> finalJointMatrices;
};