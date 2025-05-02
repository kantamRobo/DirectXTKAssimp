
// HLSL�Fregister(b0) �ɕϊ��s��Aregister(b1) �ɃI�t�Z�b�g
cbuffer TransformCB : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

cbuffer OffsetCB : register(b1)
{
    float4 offset;
    // �K�v�Ȃ�p�f�B���O
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(float3 inPos : POSITION, float4 inColor : COLOR)
{
    VSOutput outV;


    //���W�ϊ�����肭�o�C���h����Ă��Ȃ�(�V�F�[�_�[�̃o�C���h���̂͏o���Ă���)
    float4 worldPosition = mul(float4(inPos, 1.0f), World);
    float4 viewPosition = mul(worldPosition, View);
    float4 projPosition = mul(viewPosition, Projection);

    outV.position = projPosition;
    outV.color = inColor;

    return outV;
}
