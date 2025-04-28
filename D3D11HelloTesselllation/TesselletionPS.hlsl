// Tessellation.hlsl

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

// DS: �o���Z���^���b�N���
struct DSOut
{
    float4 Pos : SV_POSITION;
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

// PS: �P�F
float4 PSMain(DSOut i) : SV_Target
{
    // ���W��F�Ƀ}�b�s���O
    float3 c = i.Pos.xyz * 0.5f + 0.5f;
    return float4(c, 1.0f);
}
