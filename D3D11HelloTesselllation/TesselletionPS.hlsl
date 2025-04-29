// Tessellation.hlsl
/*
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

// DS: �o���Z���^���b�N���
struct DSOut
{
    float4 Pos : SV_POSITION;
};

// PS: �P�F
float4 PSMain(DSOut i) : SV_Target
{
    // ���W��F�Ƀ}�b�s���O
    float3 c = i.Pos.xyz * 0.5f + 0.5f;
    return float4(c, 1.0f);
}
*/
/*
// DS: �o���Z���^���b�N���
struct DSOut
{
    float4 Pos : SV_POSITION;
};

// TesselletionPS.hlsl
float4 PSMain(DSOut i) : SV_Target
{
    return float4(1,0,0,1);  // �^����
}

//�uVS��HS��DS��PS��RS��OM�v�̗���͐����������Ă��邱�Ƃ��m��
*/

// TesselletionPS.hlsl
struct DSOut
{
    float4 Pos : SV_POSITION;
};

float4 PSMain(DSOut i) : SV_Target
{
    // NDC���W�i-1�`+1�j�� 0�`1 �ɐ��K��
    float3 c = i.Pos.xyz * 0.5f + 0.5f;
    return float4(c, 1.0f);
}
