cbuffer PolyConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};


Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VSInput
{
    float3 Pos : POSITION;
    float4 Color : COLOR0; // 今回は未使用（残してOK）
    float2 Tex : TEXCOORD0;
};
struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 Tex : TEXCOORD0; // kept for VS-PS IO compatibility
   
};