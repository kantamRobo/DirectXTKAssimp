struct VS_INPUT
{
    float4 pos : POSITION; // ���_�ʒu
    float3 normal : NORMAL; // �@��
    float2 tex : TEXCOORD0; // �e�N�X�`�����W
};
 
cbuffer ConstantBuffer
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION; // �o�͂̃X�N���[�����W
    float3 normal : NORMAL; // �o�̖͂@��
    float2 tex : TEXCOORD0; // �o�͂̃e�N�X�`�����W
};