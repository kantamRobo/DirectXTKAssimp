
struct VS_INPUT
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};
 
cbuffer ConstantBuffer
{
    float4x4 World;         //���[���h�ϊ��s��
    float4x4 View;          //�r���[�ϊ��s��
    float4x4 Projection;    //�����ˉe�ϊ��s��
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