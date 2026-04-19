#ifndef PATHTRACER_SHARED_HLSLI
#define PATHTRACER_SHARED_HLSLI

static const uint MATERIAL_LAMBERTIAN = 0;
static const uint MATERIAL_METAL = 1;
static const uint MATERIAL_DIELECTRIC = 2;
static const uint MATERIAL_EMISSIVE = 3;

struct SphereData
{
    float3 center;
    float radius;
    uint materialIndex;
    uint _pad0;
    uint _pad1;
    uint _pad2;
};

struct TriangleData
{
    float3 v0;
    float _pad0;
    float3 v1;
    float _pad1;
    float3 v2;
    float _pad2;
    float3 normal;
    uint materialIndex;
};

struct MaterialData
{
    float3 albedo;
    uint type;
    float fuzz;
    float refractiveIndex;
    float emissiveStrength;
    float _pad0;
    float3 emissiveColor;
    float _pad1;
};

struct BVHNodeData
{
    float3 boundsMin;
    uint leftChild;
    float3 boundsMax;
    uint rightChild;
    uint startIndex;
    uint primitiveCount;
    uint isLeaf;
    uint _pad0;
};

cbuffer PathTracerCB : register(b0)
{
    float3 g_cameraPosition;
    float g_frameIndex;
    float3 g_cameraForward;
    float g_resetAccumulation;
    float3 g_cameraRight;
    float g_verticalFov;
    float3 g_cameraUp;
    float g_aspectRatio;
    uint g_width;
    uint g_height;
    uint g_sphereCount;
    uint g_triangleCount;
    uint g_materialCount;
    uint g_maxBounces;
    uint g_bvhNodeCount;
    uint g_randomSeed;
    float g_elapsedTimeSeconds;
    float g_russianRouletteMin;
    float g_epsilon;
    float g_padFinal;
};

#endif
