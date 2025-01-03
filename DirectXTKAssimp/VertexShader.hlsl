struct VS_INPUT
{
    float4 pos : POSITION;       // ���_�ʒu
    float3 normal : NORMAL;      // �@��
    float2 tex : TEXCOORD0;      // �e�N�X�`�����W
    float4 boneWeights : BLENDWEIGHT; // �{�[���̃E�F�C�g
    uint4 boneIndices : BLENDINDICES; // �{�[���̃C���f�b�N�X
};

cbuffer ConstantBuffer
{
    float4x4 World;              // ���[���h�ϊ��s��
    float4x4 View;               // �r���[�ϊ��s��
    float4x4 Projection;         // �����ˉe�ϊ��s��
};

cbuffer BoneBuffer
{
    float4x4 BoneTransforms[64]; // �{�[���ϊ��s��i�ő�64�{�[��������j
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;    // �o�͂̃X�N���[�����W
    float3 normal : NORMAL;      // �o�̖͂@��
    float2 tex : TEXCOORD0;      // �o�͂̃e�N�X�`�����W
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // �X�L�j���O�v�Z
    float4 skinnedPosition = float4(0.0, 0.0, 0.0, 0.0);
    float3 skinnedNormal = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 4; i++) // �ő�4�̃{�[�����T�|�[�g
    {
        if (input.boneWeights[i] > 0.0f)
        {
            float4x4 boneTransform = BoneTransforms[input.boneIndices[i]];
            skinnedPosition += mul(input.pos, boneTransform) * input.boneWeights[i];
            skinnedNormal += mul(float4(input.normal, 0.0f), boneTransform).xyz * input.boneWeights[i];
        }
    }

    // ���[���h�ϊ��A�r���[�ϊ��A�v���W�F�N�V�����ϊ���K�p
    float4 worldPosition = mul(skinnedPosition, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // �@�������[���h���W�n�ɕϊ�
    output.normal = normalize(mul(float4(skinnedNormal, 0.0f), World).xyz);

    // �e�N�X�`�����W�͂��̂܂܏o��
    output.tex = input.tex;

    return output;
}
