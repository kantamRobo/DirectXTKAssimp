#include "Model.hlsli"

// �T���v���[�X�e�[�g�ƃe�N�X�`�����\�[�X
Texture2D tex : register(t0);           // �e�N�X�`�����\�[�X (�V�F�[�_�[���\�[�X�r���[����o�C���h)
SamplerState texSampler : register(s0); // �T���v���[�X�e�[�g (�T���v�����O���@�𐧌�)

// �s�N�Z���V�F�[�_�[
float4 main(VS_OUTPUT input) : SV_TARGET
{
    // �e�N�X�`�����T���v�����O���ĐF���擾
    float4 texColor = tex.Sample(texSampler, input.tex);

// �e�N�X�`���̐F�����̂܂܏o��
return texColor;
}