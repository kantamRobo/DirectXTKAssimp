struct PolyVS_INPUT
{
    float4 pos : POSITION; // ���_�ʒu
    float3 normal : NORMAL; // �@��
    //float2 tex : TEXCOORD0; // �e�N�X�`�����W
};

cbuffer PolyConstantBuffer
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};

struct PolyVS_OUTPUT
{
    float4 pos : SV_POSITION; // �o�͂̃X�N���[�����W
    float3 normal : NORMAL; // �o�̖͂@��
    float2 tex : TEXCOORD0; // �o�͂̃e�N�X�`�����W
};
PolyVS_OUTPUT main(PolyVS_INPUT input)
{
    PolyVS_OUTPUT output;

    // ���[���h�ϊ��A�r���[�ϊ��A�v���W�F�N�V�����ϊ���K�p
    float4 worldPosition = mul(input.pos, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // �@���ƃe�N�X�`�����W�͂��̂܂܏o��
    output.normal = mul(float4(input.normal, 0.0f), World).xyz; // �@�������[���h���W�n�ɕϊ�
   // output.tex = input.tex;

    return output;
}