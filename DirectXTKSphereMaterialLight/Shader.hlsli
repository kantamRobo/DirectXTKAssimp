cbuffer PolyConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

// VS "Input" : IAと一致するセマンティクスだけ
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0; // 今回PSでは未使用でもOK
};

// VS "Output" : ここで初めてSV_Positionにする
struct VSOutput
{
    float4 position : SV_Position;
    float3 Nrm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

#define Roughness (Params.x)
#define Metallic  (Params.y)
#define Opacity   (Params.z)

cbuffer MaterialCB : register(b1)
{
    float4 BaseColor;
    float3 Emissive;
    float _pad0;
    float4 Params;
};

cbuffer DirectionLightCb : register(b2)
{
    float4 ligDirection;
    float4 ligColor;
    float4 eyePos;
}
