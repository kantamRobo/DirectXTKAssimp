
struct VS_INPUT
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};
 
cbuffer ConstantBuffer
{
    float4x4 World;         //ワールド変換行列
    float4x4 View;          //ビュー変換行列
    float4x4 Projection;    //透視射影変換行列
}
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = input.pos;
    output.normal = input.normal;
    output.tex = input.tex;
    return output;
}