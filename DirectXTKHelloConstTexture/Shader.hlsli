// ���� Shader.hlsli ����������������������������������������������������
cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
};

struct VSOutput        // �� �gPSInput�h �ł͂Ȃ��p�r�������閼�O��
{
    float4 position : SV_POSITION; // ���K�{
    float4 color : COLOR;
};
