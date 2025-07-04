// ���� VertexShader.hlsl ������������������������������������������
#include "Shader.hlsli"

VSOutput main(float3 inPos : POSITION,
    float4 inColor : COLOR)
{
    VSOutput outV;

    float4 pos = float4(inPos, 1.0f);
    outV.position = pos + offset; // �� SV_POSITION ��K������
    outV.color = inColor;

    return outV;
}
