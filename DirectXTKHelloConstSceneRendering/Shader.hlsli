cbuffer PolyConstantBuffer
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};
// ���� Shader.hlsli ����������������������������������������������������
cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
};

// ���� PixelShader.hlsl 
struct VSOutput        // �� �gPSInput�h �ł͂Ȃ��p�r�������閼�O��
{
    float4 position : SV_POSITION; // ���K�{
    float4 color : COLOR;
};
