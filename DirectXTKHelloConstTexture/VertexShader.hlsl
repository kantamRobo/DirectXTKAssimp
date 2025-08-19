// ���� VertexShader.hlsl ������������������������������������������
#include "Shader.hlsli"

VSOutput main(VSOutput i)
{
    VSOutput outV;


    //���W�ϊ�����肭�o�C���h����Ă��Ȃ�(�V�F�[�_�[�̃o�C���h���̂͏o���Ă���)
    float4 worldPosition = mul(i.position, World);
    float4 viewPosition = mul(worldPosition, View);
    float4 projPosition = mul(viewPosition, Projection);

    outV.position = projPosition;
   

  
   
    outV.Nrm = i.Nrm;
    outV.Tex = i.Tex;
    return outV;
}

