
cbuffer TessFactorBuffer : register(b0)
{
    float Inner;    // SV_InsideTessFactor �ɑΉ�
    float Outer;    // SV_TessFactor �ɑΉ��i3 �v�f���ׂĂɓ����l�𗬂��z��j
};

// VS: �P�ɓ��͒��_�����֓n��
struct VSOut
{
    float3 Pos : POSITION;
};
VSOut VSMain(uint vid : SV_VertexID)
{
    VSOut o;
    // ���_�o�b�t�@���玩���擾
    o.Pos = float3(0, 0, 0);
    // SV_VertexID �Œ��_�o�b�t�@�� indexing ����ꍇ��
    // StructuredBuffer �����g���K�v������܂����A
    // �{�T���v���ł� IASetVertexBuffers �œn���P���o�b�t�@���g�p���܂��B
    // HLSL �ł͒��_�o�b�t�@�����ڌ����Ȃ����߁A���̊֐��ł͎g���܂���B
    return o;
}