struct VS_INPUT
{
    /*
    float4 pos : POSITION; // ���_�ʒu
    float3 normal : NORMAL; // �@��
    float2 tex : TEXCOORD0; // �e�N�X�`�����W
*/
    
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1; // �ǉ�
};
 
cbuffer ConstantBuffer
{
    float4x4 World; // ���[���h�ϊ��s��
    float4x4 View; // �r���[�ϊ��s��
    float4x4 Projection; // �����ˉe�ϊ��s��
};
cbuffer PointLightBuffer : register(b1)
{
    float3 lightPos; // �|�C���g���C�g�̈ʒu�i���[���h���W�n�j
    float intensity; // ���̋���

    float3 lightColor; // ���̐F
    float range; // �L���͈�

    float3 attenuation; // �����p�����[�^
    float padding; // �p�f�B���O
};
struct VS_OUTPUT
{
    
    /*
    float4 pos : SV_POSITION; // �o�͂̃X�N���[�����W
    float3 normal : NORMAL; // �o�̖͂@��
    float2 tex : TEXCOORD0; // �o�͂̃e�N�X�`�����W

*/
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1; // �ǉ�
};