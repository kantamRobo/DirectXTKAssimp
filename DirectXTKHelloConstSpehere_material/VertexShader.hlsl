// ── VertexShader.hlsl ─────────────────────
#include "Shader.hlsli"

VSOutput main(VSOutput i)
{
    VSOutput outV;


    //座標変換が上手くバインドされていない(シェーダーのバインド自体は出来ている)
    float4 worldPosition = mul(i.position, World);
    float4 viewPosition = mul(worldPosition, View);
    float4 projPosition = mul(viewPosition, Projection);

    outV.position = projPosition;
   

  
    outV.Tex = i.Tex;
    return outV;
}

