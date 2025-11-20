// „Ÿ„Ÿ VertexShader.hlsl „Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ
#include "Shader.hlsli"

VSOutput main(VSOutput i)
{
    VSOutput outV;

    
   
    float4 worldPosition = mul(i.position, World);
    float4 viewPosition = mul(worldPosition, View);
    float4 projPosition = mul(viewPosition, Projection);
    
    
    outV.position = projPosition;
   

  
   
  
    outV.Tex = i.Tex;
    return outV;
}

