cbuffer PolyConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 Nrm : NORMAL;
    float2 Tex : TEXCOORD0; // kept for VS-PS IO compatibility
    float3 worldPos : TEXCOORD1;
};

#define Roughness (Params.x)
#define Metallic  (Params.y)
#define Opacity   (Params.z)

cbuffer MaterialCB : register(b1)
{
    float4 BaseColor; // RGBA
    float3 Emissive;
    float _pad0;
    float4 Params; // x=Roughness, y=Metallic, z=Opacity, w=Unused
};

cbuffer DirectionLightCb : register(b2)
{
    float4 ligDirection; // xyz used
    float4 ligColor; // rgb used
    float4 eyePos; // xyz used
};
