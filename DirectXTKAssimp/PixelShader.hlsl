#include "Model.hlsli"
// �T���v���[�X�e�[�g�ƃV�F�[�_�[���\�[�X�r���[ (�s�N�Z���V�F�[�_�[�p)
Texture2D tex : register(t0);        // �e�N�X�`�����\�[�X
SamplerState texSampler : register(s0); // �T���v���[


float4 main(PSInput input) : SV_TARGET{
    // �e�N�X�`�����T���v�����O���A���_�J���[�Ə�Z
    float4 texColor = tex.Sample(texSampler, input.texcoord);
    return texColor * input.color;
}