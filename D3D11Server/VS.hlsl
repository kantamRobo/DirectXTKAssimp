// ���_�V�F�[�_�[: ���͂� float3 �̈ʒu����
struct VSIn
{
    float3 pos : POSITION;
};

struct PSIn
{
    float4 pos : SV_POSITION;
};

PSIn VSMain(VSIn input)
{
    PSIn o;
    // ���̂܂܃N���b�v��Ԃɏo�́iNDC�ϊ��ςݍ��W�z��j
    o.pos = float4(input.pos, 1.0f);
    return o;
}
