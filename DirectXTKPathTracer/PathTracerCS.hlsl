#include "PathTracerShared.hlsli"

StructuredBuffer<SphereData> g_spheres : register(t0);
StructuredBuffer<TriangleData> g_triangles : register(t1);
StructuredBuffer<MaterialData> g_materials : register(t2);
StructuredBuffer<BVHNodeData> g_bvhNodes : register(t3);

RWTexture2D<float4> g_accumulation : register(u0);
RWTexture2D<float4> g_output : register(u1);

struct Ray
{
    float3 origin;
    float3 direction;
};

struct HitRecord
{
    float t;
    float3 position;
    float3 normal;
    uint materialIndex;
    bool frontFace;
};

uint Hash(uint state)
{
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}

float RandomFloat(inout uint state)
{
    state = Hash(state);
    return (state & 0x00FFFFFFu) / 16777216.0;
}

float3 RandomInUnitSphere(inout uint rng)
{
    [loop]
    for (int i = 0; i < 32; ++i)
    {
        const float3 p = 2.0f * float3(RandomFloat(rng), RandomFloat(rng), RandomFloat(rng)) - 1.0f;
        if (dot(p, p) < 1.0f)
        {
            return p;
        }
    }
    return float3(0.0f, 1.0f, 0.0f);
}

float3 RandomUnitVector(inout uint rng)
{
    return normalize(RandomInUnitSphere(rng));
}

float3 Reflect(const float3 v, const float3 n)
{
    return v - 2.0f * dot(v, n) * n;
}

float3 RefractVec(const float3 uv, const float3 n, float etaRatio)
{
    const float cosTheta = min(dot(-uv, n), 1.0f);
    const float3 rOutPerp = etaRatio * (uv + cosTheta * n);
    const float3 rOutParallel = -sqrt(abs(1.0f - dot(rOutPerp, rOutPerp))) * n;
    return rOutPerp + rOutParallel;
}

float Schlick(float cosine, float refIdx)
{
    float r0 = (1.0f - refIdx) / (1.0f + refIdx);
    r0 *= r0;
    return r0 + (1.0f - r0) * pow(1.0f - cosine, 5.0f);
}

bool HitSphere(Ray ray, SphereData sphere, float tMin, float tMax, inout HitRecord rec)
{
    const float3 oc = ray.origin - sphere.center;
    const float a = dot(ray.direction, ray.direction);
    const float halfB = dot(oc, ray.direction);
    const float c = dot(oc, oc) - sphere.radius * sphere.radius;
    const float discriminant = halfB * halfB - a * c;

    if (discriminant < 0.0f)
    {
        return false;
    }

    const float sqrtd = sqrt(discriminant);
    float root = (-halfB - sqrtd) / a;
    if (root < tMin || root > tMax)
    {
        root = (-halfB + sqrtd) / a;
        if (root < tMin || root > tMax)
        {
            return false;
        }
    }

    rec.t = root;
    rec.position = ray.origin + rec.t * ray.direction;
    const float3 outwardNormal = (rec.position - sphere.center) / sphere.radius;
    rec.frontFace = dot(ray.direction, outwardNormal) < 0.0f;
    rec.normal = rec.frontFace ? outwardNormal : -outwardNormal;
    rec.materialIndex = sphere.materialIndex;
    return true;
}

bool HitTriangle(Ray ray, TriangleData tri, float tMin, float tMax, inout HitRecord rec)
{
    const float3 edge1 = tri.v1 - tri.v0;
    const float3 edge2 = tri.v2 - tri.v0;
    const float3 pvec = cross(ray.direction, edge2);
    const float det = dot(edge1, pvec);

    if (abs(det) < 1e-8f)
    {
        return false;
    }

    const float invDet = 1.0f / det;
    const float3 tvec = ray.origin - tri.v0;
    const float u = dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    const float3 qvec = cross(tvec, edge1);
    const float v = dot(ray.direction, qvec) * invDet;
    if (v < 0.0f || (u + v) > 1.0f)
    {
        return false;
    }

    const float t = dot(edge2, qvec) * invDet;
    if (t < tMin || t > tMax)
    {
        return false;
    }

    rec.t = t;
    rec.position = ray.origin + t * ray.direction;
    const float3 n = normalize(tri.normal);
    rec.frontFace = dot(ray.direction, n) < 0.0f;
    rec.normal = rec.frontFace ? n : -n;
    rec.materialIndex = tri.materialIndex;
    return true;
}

bool HitWorldBruteForce(Ray ray, float tMin, float tMax, inout HitRecord rec)
{
    bool hitAnything = false;
    float closest = tMax;
    HitRecord temp;

    [loop]
    for (uint i = 0; i < g_sphereCount; ++i)
    {
        if (HitSphere(ray, g_spheres[i], tMin, closest, temp))
        {
            hitAnything = true;
            closest = temp.t;
            rec = temp;
        }
    }

    [loop]
    for (uint i = 0; i < g_triangleCount; ++i)
    {
        if (HitTriangle(ray, g_triangles[i], tMin, closest, temp))
        {
            hitAnything = true;
            closest = temp.t;
            rec = temp;
        }
    }

    return hitAnything;
}

float3 SkyColor(float3 dir)
{
    const float t = 0.5f * (normalize(dir).y + 1.0f);
    return lerp(float3(1.0f, 1.0f, 1.0f), float3(0.5f, 0.7f, 1.0f), t);
}

float3 RayColor(Ray ray, inout uint rng)
{
    float3 throughput = 1.0f.xxx;
    float3 radiance = 0.0f.xxx;

    [loop]
    for (uint bounce = 0; bounce < g_maxBounces; ++bounce)
    {
        HitRecord rec;
        if (!HitWorldBruteForce(ray, g_epsilon, 1e30f, rec))
        {
            radiance += throughput * SkyColor(ray.direction);
            break;
        }

        const MaterialData mat = g_materials[min(rec.materialIndex, g_materialCount - 1)];

        if (mat.type == MATERIAL_EMISSIVE)
        {
            radiance += throughput * mat.emissiveColor * mat.emissiveStrength;
            break;
        }

        float3 scatterDirection;

        if (mat.type == MATERIAL_LAMBERTIAN)
        {
            scatterDirection = normalize(rec.normal + RandomUnitVector(rng));
            if (all(abs(scatterDirection) < 1e-6f.xxx))
            {
                scatterDirection = rec.normal;
            }
            throughput *= mat.albedo;
        }
        else if (mat.type == MATERIAL_METAL)
        {
            scatterDirection = normalize(Reflect(normalize(ray.direction), rec.normal) + mat.fuzz * RandomInUnitSphere(rng));
            if (dot(scatterDirection, rec.normal) <= 0.0f)
            {
                break;
            }
            throughput *= mat.albedo;
        }
        else
        {
            const float refractionRatio = rec.frontFace ? (1.0f / max(mat.refractiveIndex, 1e-4f)) : mat.refractiveIndex;
            const float3 unitDir = normalize(ray.direction);
            const float cosTheta = min(dot(-unitDir, rec.normal), 1.0f);
            const float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));

            const bool cannotRefract = refractionRatio * sinTheta > 1.0f;
            const float reflectProb = Schlick(cosTheta, refractionRatio);
            if (cannotRefract || reflectProb > RandomFloat(rng))
            {
                scatterDirection = Reflect(unitDir, rec.normal);
            }
            else
            {
                scatterDirection = RefractVec(unitDir, rec.normal, refractionRatio);
            }
            throughput *= mat.albedo;
        }

        if (any(!isfinite(throughput)))
        {
            break;
        }

        if (bounce > 2)
        {
            const float survive = max(max(throughput.r, throughput.g), max(throughput.b, g_russianRouletteMin));
            if (RandomFloat(rng) > survive)
            {
                break;
            }
            throughput /= max(survive, 1e-3f);
        }

        ray.origin = rec.position + rec.normal * g_epsilon;
        ray.direction = normalize(scatterDirection);
    }

    return radiance;
}

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    if (dtid.x >= g_width || dtid.y >= g_height)
    {
        return;
    }

    uint rng = g_randomSeed ^ (dtid.x * 1973u + dtid.y * 9277u + uint(g_frameIndex) * 26699u);

    const float2 jitter = float2(RandomFloat(rng), RandomFloat(rng));
    const float2 uv = (float2(dtid.xy) + jitter) / float2(g_width, g_height);
    const float2 ndc = float2(2.0f * uv.x - 1.0f, 1.0f - 2.0f * uv.y);

    const float tanHalfFov = tan(0.5f * g_verticalFov);
    const float3 rayDir = normalize(
        g_cameraForward +
        ndc.x * g_aspectRatio * tanHalfFov * g_cameraRight +
        ndc.y * tanHalfFov * g_cameraUp);

    Ray ray;
    ray.origin = g_cameraPosition;
    ray.direction = rayDir;

    float3 sampleColor = RayColor(ray, rng);

    if (any(!isfinite(sampleColor)))
    {
        sampleColor = 0.0f.xxx;
    }

    float4 accum = g_resetAccumulation > 0.5f ? 0.0f.xxxx : g_accumulation[dtid.xy];
    accum.rgb += sampleColor;
    accum.a += 1.0f;
    g_accumulation[dtid.xy] = accum;

    const float invSamples = 1.0f / max(accum.a, 1.0f);
    g_output[dtid.xy] = float4(accum.rgb * invSamples, 1.0f);
}
