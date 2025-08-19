cbuffer PolyConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

// VS "Input"
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

// VS "Output"
struct VSOutput
{
    float4 position : SV_Position;
    float3 Nrm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

// 既存パラメータ
#define Roughness (Params.x)
#define Metallic  (Params.y)
#define Opacity   (Params.z)

// ★ Disney拡張パラメータ
#define Specular       (Params2.x)
#define SpecularTint   (Params2.y)
#define Sheen          (Params2.z)
#define SheenTint      (Params2.w)
#define Clearcoat      (Params3.x)
#define ClearcoatGloss (Params3.y)   // ← ← ここ、"名前(…)" ではなく「名前 ? (…)"
#define Subsurface     (Params3.z)

cbuffer MaterialCB : register(b1)
{
    float4 BaseColor; // RGBA
    float3 Emissive;
    float _pad0;
    float4 Params; // x=Roughness, y=Metallic, z=Opacity, w=Unused
    float4 Params2; // x=Specular, y=SpecularTint, z=Sheen, w=SheenTint
    float4 Params3; // x=Clearcoat, y=ClearcoatGloss, z=Subsurface, w=Unused
};

cbuffer DirectionLightCb : register(b2)
{
    float4 ligDirection;
    float4 ligColor;
    float4 eyePos;
}; // ← セミコロン必須
