

cbuffer PolyConstantBuffer:register(b0)
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};

struct VSOutput        // �� �gPSInput�h �ł͂Ȃ��p�r�������閼�O��
{
    float4 position : SV_Position; // ���K�{
    float4 color : COLOR;
    float3 Nrm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

#define Roughness (Params.x)
#define Metallic  (Params.y)
#define Opacity   (Params.z)

// Material.hlsli
cbuffer MaterialCB : register(b1)
{
    float4 BaseColor; // RGBA
    float3 Emissive;
    float _pad0; // 16B���E���킹
    float4 Params; // x=Roughness, y=Metallic, z=Opacity, w=Unused
}

// �f�B���N�V�������C�g�p�̃f�[�^���󂯎�邽�߂̒萔�o�b�t�@�[��p�ӂ���
cbuffer DirectionLightCb : register(b2)
{
    float3 ligDirection; //���C�g�̕���
    float3 ligColor; //���C�g�̃J���[

    // step-3 ���_�̃f�[�^�ɃA�N�Z�X���邽�߂̕ϐ���萔�o�b�t�@�[�ɒǉ�����
    float3 eyePos; //���_�̈ʒu
};


///////////////////////////////////////////
// �V�F�[�_�[���\�[�X
///////////////////////////////////////////
// ���f���e�N�X�`��
Texture2D<float4> g_texture : register(t0);

///////////////////////////////////////////
// �T���v���[�X�e�[�g
///////////////////////////////////////////
sampler g_sampler : register(s0);