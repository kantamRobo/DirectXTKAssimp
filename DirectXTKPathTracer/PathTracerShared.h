#pragma once

#include <cstdint>

namespace pt
{
    static constexpr uint32_t kMaxBounces = 8;
    static constexpr uint32_t kMaxSpheres = 16;
    static constexpr uint32_t kMaxTriangles = 2048;
    static constexpr uint32_t kMaxMaterials = 32;
    static constexpr uint32_t kMaxBVHNodes = 4096;

    enum MaterialType : uint32_t
    {
        MaterialLambertian = 0,
        MaterialMetal = 1,
        MaterialDielectric = 2,
        MaterialEmissive = 3,
    };

    struct float3_packed
    {
        float x;
        float y;
        float z;
    };

    struct SphereData
    {
        float3_packed center;
        float radius;
        uint32_t materialIndex;
        uint32_t _pad0;
        uint32_t _pad1;
        uint32_t _pad2;
    };

    struct TriangleData
    {
        float3_packed v0;
        float _pad0;
        float3_packed v1;
        float _pad1;
        float3_packed v2;
        float _pad2;
        float3_packed normal;
        uint32_t materialIndex;
    };

    struct MaterialData
    {
        float3_packed albedo;
        uint32_t type;
        float fuzz;
        float refractiveIndex;
        float emissiveStrength;
        float _pad0;
        float3_packed emissiveColor;
        float _pad1;
    };

    struct BVHNodeData
    {
        float3_packed boundsMin;
        uint32_t leftChild;
        float3_packed boundsMax;
        uint32_t rightChild;
        uint32_t startIndex;
        uint32_t primitiveCount;
        uint32_t isLeaf;
        uint32_t _pad0;
    };

    struct PathTracerCB
    {
        float3_packed cameraPosition;
        float frameIndex;
        float3_packed cameraForward;
        float resetAccumulation;
        float3_packed cameraRight;
        float verticalFov;
        float3_packed cameraUp;
        float aspectRatio;
        uint32_t width;
        uint32_t height;
        uint32_t sphereCount;
        uint32_t triangleCount;
        uint32_t materialCount;
        uint32_t maxBounces;
        uint32_t bvhNodeCount;
        uint32_t randomSeed;
        float elapsedTimeSeconds;
        float russianRouletteMin;
        float epsilon;
        float _padFinal;
    };
}
