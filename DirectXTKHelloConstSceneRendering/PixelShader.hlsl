// ���� PixelShader.hlsl ��������������������������������������������
#include "Shader.hlsli"
struct VSOutput        // �� �gPSInput�h �ł͂Ȃ��p�r�������閼�O��
{
    float4 position : SV_POSITION; // ���K�{
    float4 color : COLOR;
};

float4 main(VSOutput input) : SV_TARGET   // �G���g������ "main" �ɓ���
{
    return input.color;
}
