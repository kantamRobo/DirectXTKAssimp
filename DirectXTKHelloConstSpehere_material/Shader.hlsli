cbuffer PolyConstantBuffer
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};

struct VSOutput        // �� �gPSInput�h �ł͂Ȃ��p�r�������閼�O��
{
    float4 position : SV_Position; // ���K�{
    float4 color : COLOR;
    float3 Nrm : NORMAL;
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
    float _pad0; // 16B���E���킹
    float4 Params; // x=Roughness, y=Metallic, z=Opacity, w=Unused
}
