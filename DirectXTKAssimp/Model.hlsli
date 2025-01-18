

// ���_�V�F�[�_�[�̏o��
struct PSInput
{
    float4 position : SV_POSITION; // �N���b�v��Ԃ̈ʒu
    float4 color : COLOR; // ���_�J���[
    float2 texcoord : TEXCOORD; // �e�N�X�`�����W
};

struct VS_INPUT
{
    float4 pos : POSITION; // ���_�ʒu
    float3 normal : NORMAL; // �@��
    float2 tex : TEXCOORD0; // �e�N�X�`�����W
   // float4 boneWeights : BLENDWEIGHT; // �{�[���̃E�F�C�g
   // uint4 boneIndices : BLENDINDICES; // �{�[���̃C���f�b�N�X
};

cbuffer ConstantBuffer
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};

cbuffer BoneBuffer
{
    float4x4 BoneTransforms[64]; // �{�[���ϊ��s��i�ő�64�{�[��������j
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION; // �o�͂̃X�N���[�����W
    float3 normal : NORMAL; // �o�̖͂@��
    float2 tex : TEXCOORD0; // �o�͂̃e�N�X�`�����W
};
