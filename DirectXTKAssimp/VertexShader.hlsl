#include "Model.hlsli"

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // ���[���h�ϊ��A�r���[�ϊ��A�v���W�F�N�V�����ϊ���K�p
    float4 worldPosition = mul(input.pos, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // �@���ƃe�N�X�`�����W�͂��̂܂܏o��
    output.normal = mul(float4(input.normal, 0.0f), World).xyz; // �@�������[���h���W�n�ɕϊ�
    output.tex = input.tex;

    return output;
}
