// ���� PixelShader.hlsl ��������������������������������������������
#include "Shader.hlsli"

float4 main(VSOutput input) : SV_TARGET   // �G���g������ "main" �ɓ���
{
    return input.color;
}
