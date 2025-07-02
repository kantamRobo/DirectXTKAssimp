#include "Model.hlsli"

// �T���v���[�X�e�[�g�ƃe�N�X�`�����\�[�X
Texture2D tex : register(t0);           // �e�N�X�`�����\�[�X (�V�F�[�_�[���\�[�X�r���[����o�C���h)
SamplerState texSampler : register(s0); // �T���v���[�X�e�[�g (�T���v�����O���@�𐧌�)

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // �e�N�X�`���F
    float4 texColor = tex.Sample(texSampler, input.tex);

    // ���C�g����
    float3 toLight = lightPos - input.worldPos;
    float distance = length(toLight);
    toLight = normalize(toLight);

    // �����v�Z
    float att = 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);

    // Lambert����
    float NdotL = max(dot(normalize(input.normal), toLight), 0.0f);

    // Diffuse�v�Z
    float3 diffuse = texColor.rgb * lightColor * NdotL * intensity * att;

    // �o��
    return float4(diffuse, texColor.a);
}
