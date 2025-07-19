// ���� PixelShader.hlsl ��������������������������������������������
#include "Shader.hlsli"

// �e�N�X�`������уT���v���[�X�e�[�g���`
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

// ���͍\���́iUV���W���󂯎���悤�ɂ���j
struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    
};

// �s�N�Z���V�F�[�_�[
float4 main(PSInput input) : SV_TARGET
{
    // �e�N�X�`�����T���v�����O���ĐF��Ԃ�
    return g_Texture.Sample(g_Sampler, input.uv);
}
