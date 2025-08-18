// ���� PixelShader.hlsl ��������������������������������������������
#include "Shader.hlsli"

// �e�N�X�`���Ȃ��E�萔�F�{�G�~�b�V�u�݂̂�Ԃ��ŏ�PS
float4 main(VSOutput i) : SV_Target
{
    float3 color = saturate(BaseColor.rgb + Emissive);
    return float4(color, BaseColor.a * Opacity);
}