cbuffer PolyConstantBuffer
{
    float4x4 World; // ワールド変換行列
    float4x4 View; // ビュー変換行列
    float4x4 Projection; // 透視射影変換行列
};

struct VSInput
{
    float3 Pos : POSITION;
    float4 Color : COLOR0; // 今回は未使用（残してOK）
    float2 Tex : TEXCOORD0;
};

struct VSOutput
{
    float4 Pos : SV_Position;
    float2 Tex : TEXCOORD0;
};


#define Roughness (Params.x)
#define Metallic  (Params.y)
#define Opacity   (Params.z)

// Material.hlsli
cbuffer MaterialCB : register(b1)
{
    float4 BaseColor; // RGBA
    float3 Emissive;
    float _pad0; // 16B境界合わせ
    float4 Params; // x=Roughness, y=Metallic, z=Opacity, w=Unused
}
