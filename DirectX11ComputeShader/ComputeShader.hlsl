// �\���̂̒�`
struct RWStructuredBuffer
{
    uint index;
    float value;
};

// �ǂݏ����o�b�t�@
RWStructuredBuffer<MyData> bufferData : register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    // �C���f�b�N�X�ɑΉ�����v�f��ύX
    bufferData[DTid.x].value += 1.0f;
}
