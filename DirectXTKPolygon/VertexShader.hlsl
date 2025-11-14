// „Ÿ„Ÿ VertexShader.hlsl „Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ
#include "Shader.hlsli"

VSOutput main(float4 inPos : POSITION,
    float4 inColor : COLOR)
{
    VSOutput outV;

   
    outV.position = inPos;
   

    return outV;
}
