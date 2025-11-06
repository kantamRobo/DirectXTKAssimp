#include "Shader.hlsli"




float4 main(VSOutput i) : SV_Target
{
    // そのままテクスチャ色を返す（アルファも採用）
    return gTexture.Sample(gSampler, i.Tex);
}