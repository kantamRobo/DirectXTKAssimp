// �\���̂̒�`
struct rwStructuredBuffer
{
    unsigned int index;
    float value;
};

// �ǂݏ����o�b�t�@
RWStructuredBuffer<rwStructuredBuffer> bufferData : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // �C���f�b�N�X�ɑΉ�����v�f��ύX
    bufferData[DTid.x].value += 1.0f;
}
