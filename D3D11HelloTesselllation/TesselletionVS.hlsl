// ���̓��C�A�E�g�ɍ��킹�� POSITION ���󂯎��
struct VSIn
{
    float3 Pos : POSITION;
};
struct VSOut
{
    float3 Pos : POSITION;
};
VSOut VSMain(VSIn vin)
{
    VSOut o;
    // ������ IASetVertexBuffers �œn���ꂽ�o�b�t�@�̍��W�����̂܂܎g��
    o.Pos = vin.Pos;
    return o;
}