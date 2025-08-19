// ���� PixelShader.hlsl ��������������������������������������������
#include "Shader.hlsli"

// �e�N�X�`���Ȃ��E�萔�F�{�G�~�b�V�u�݂̂�Ԃ��ŏ�PS
float4 main(VSOutput i) : SV_Target
{
    
    // �s�N�Z���̖@���ƃ��C�g�̕����̓��ς��v�Z����
    float t = dot(i.Nrm, ligDirection);
    t *= -1.0f;

    // ���ς̌��ʂ�0�ȉ��Ȃ�0�ɂ���
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    // �g�U���ˌ������߂�
    float3 diffuseLig = ligColor * t;

    // step-4 ���˃x�N�g�������߂�
    float3 refVec = reflect(ligDirection, i.Nrm);

    // step-5 �������������T�[�t�F�C�X���王�_�ɐL�т�x�N�g�������߂�
    float3 toEye = eyePos - i.worldPos;
    toEye = normalize(toEye);

    // step-6 ���ʔ��˂̋��������߂�
    t = dot(refVec, toEye);
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    // step-7 ���ʔ��˂̋������i��
    t = pow(t, 5.0f);

    // step-8 ���ʔ��ˌ������߂�
    float3 specularLig = ligColor * t;

    // step-9 �g�U���ˌ��Ƌ��ʔ��ˌ��𑫂��Z���āA�ŏI�I�Ȍ������߂�
    float3 lig = diffuseLig + specularLig;
    float4 finalColor = g_texture.Sample(g_sampler, i.Tex);

    // step-10 �e�N�X�`���J���[�ɋ��߂�������Z���čŏI�o�̓J���[�����߂�
    finalColor.xyz *= lig;
    float3 color = saturate(BaseColor.rgb + Emissive);

    
    return float4(color, BaseColor.a * Opacity);

}

 