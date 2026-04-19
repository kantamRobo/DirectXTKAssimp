#pragma once

#include <vector>

#include "DeviceResources.h"
#include "PathTracerShared.h"

class ComputeShader
{
public:
    ComputeShader() = default;

    void Initialize(DX::DeviceResources* deviceResources, uint32_t width, uint32_t height);
    void OnResize(DX::DeviceResources* deviceResources, uint32_t width, uint32_t height);
    void OnDeviceLost();

    void SetCamera(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& forward,
        const DirectX::XMFLOAT3& right,
        const DirectX::XMFLOAT3& up,
        float verticalFovRadians,
        bool resetAccumulation);

    void Render(DX::DeviceResources* deviceResources, float elapsedSeconds);

    uint32_t GetSampleCount() const noexcept { return m_sampleCount; }
    uint32_t GetMaxBounces() const noexcept { return m_maxBounces; }

private:
    struct ScreenVertex
    {
        DirectX::XMFLOAT2 position;
        DirectX::XMFLOAT2 uv;
    };

    struct CpuMaterial
    {
        DirectX::XMFLOAT3 albedo;
        float fuzz;
        float refractiveIndex;
        float emissiveStrength;
        DirectX::XMFLOAT3 emissiveColor;
        uint32_t type;
    };

    struct CpuSphere
    {
        DirectX::XMFLOAT3 center;
        float radius;
        uint32_t materialIndex;
    };

    struct CpuTriangle
    {
        DirectX::XMFLOAT3 v0;
        DirectX::XMFLOAT3 v1;
        DirectX::XMFLOAT3 v2;
        DirectX::XMFLOAT3 normal;
        uint32_t materialIndex;
    };

    struct CpuBVHNode
    {
        DirectX::XMFLOAT3 boundsMin;
        DirectX::XMFLOAT3 boundsMax;
        uint32_t leftChild;
        uint32_t rightChild;
        uint32_t startIndex;
        uint32_t primitiveCount;
        uint32_t isLeaf;
    };

    void CreateShaders(ID3D11Device* device);
    void CreateRenderResources(ID3D11Device* device, uint32_t width, uint32_t height);
    void CreateSceneBuffers(ID3D11Device* device);
    void BuildScene();
    void BuildLinearBVH();
    void AppendIndexedTriangles(
        const std::vector<DirectX::XMFLOAT3>& vertices,
        const std::vector<uint32_t>& indices,
        uint32_t materialIndex);
    void UpdateConstantBuffer(ID3D11DeviceContext* context, DX::DeviceResources* deviceResources, float elapsedSeconds);
    void DispatchPathTrace(ID3D11DeviceContext* context);
    void CompositeToBackbuffer(ID3D11DeviceContext* context, DX::DeviceResources* deviceResources);

    template<typename T>
    static Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStructuredBuffer(
        ID3D11Device* device,
        const std::vector<T>& data,
        bool allowUav,
        bool dynamic = false);

    template<typename T>
    static void UploadStructuredBuffer(
        ID3D11DeviceContext* context,
        ID3D11Buffer* buffer,
        const std::vector<T>& data);

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShaderFromFile(
        const wchar_t* path,
        const char* entryPoint,
        const char* profile);

    DirectX::XMFLOAT3 m_cameraPosition = { 0.0f, 1.2f, -6.0f };
    DirectX::XMFLOAT3 m_cameraForward = { 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT3 m_cameraRight = { 1.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_cameraUp = { 0.0f, 1.0f, 0.0f };
    float m_verticalFov = DirectX::XM_PIDIV4;

    uint32_t m_width = 1;
    uint32_t m_height = 1;
    uint32_t m_sampleCount = 0;
    uint32_t m_randomSeed = 1;
    uint32_t m_maxBounces = pt::kMaxBounces;
    bool m_resetAccumulation = true;

    std::vector<CpuMaterial> m_materials;
    std::vector<CpuSphere> m_spheres;
    std::vector<CpuTriangle> m_triangles;
    std::vector<CpuBVHNode> m_bvhNodes;

    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_pathTracerCS;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_compositeVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_compositePS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_compositeInputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSampler;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pathTracerCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_screenVB;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_accumTexture;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_accumUAV;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_outputTexture;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_outputUAV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_outputSRV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_sphereBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sphereSRV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_triangleBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_triangleSRV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_materialSRV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_bvhBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bvhSRV;
};
