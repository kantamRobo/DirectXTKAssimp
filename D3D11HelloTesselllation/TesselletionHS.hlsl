
cbuffer TessFactorBuffer : register(b0)
{
    float Inner;    // SV_InsideTessFactor �ɑΉ�
    float Outer;    // SV_TessFactor �ɑΉ��i3 �v�f���ׂĂɓ����l�𗬂��z��j
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

// VS: �P�ɓ��͒��_�����֓n��
struct VSOut
{
    float3 Pos : POSITION;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSConstFunc")]
HSOut HSMain(InputPatch<VSOut, 3> patch, uint cid : SV_OutputControlPointID)
{
    HSOut o;
    o.Pos = patch[cid].Pos;
    return o;
}

HSConst HSConstFunc(InputPatch<VSOut, 3> patch)
{
    HSConst o;
    o.EdgeTess[0] = Outer;
    o.EdgeTess[1] = Outer;
    o.EdgeTess[2] = Outer;
    o.InsideTess = Inner;
    return o;
}