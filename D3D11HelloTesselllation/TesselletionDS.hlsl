
// DS: �o���Z���^���b�N���
struct DSOut
{
    float4 Pos : SV_POSITION;
};
// HS: 3 control-point patch ���󂯎��A�o�͐���_�ƃp�b�`�萔�𐶐�
struct HSOut
{
    float3 Pos : POSITION;
};

struct HSConst
{
    float EdgeTess[3]     : SV_TessFactor;       // �O��
    float InsideTess : SV_InsideTessFactor; // ����
};

[domain("tri")]
DSOut DSMain(HSConst hc, const OutputPatch<HSOut, 3> patch, float3 bary : SV_DomainLocation)
{
    DSOut o;
    float3 p = patch[0].Pos * bary.x
        + patch[1].Pos * bary.y
        + patch[2].Pos * bary.z;
    // ���̂܂܃N���b�v��ԂƉ���i�ˉe�s��Ȃǂ͏ȗ��j
    o.Pos = float4(p, 1.0f);
    return o;
}