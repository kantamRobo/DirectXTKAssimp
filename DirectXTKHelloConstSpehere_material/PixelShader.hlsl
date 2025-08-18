// ── PixelShader.hlsl ──────────────────────
#include "Shader.hlsli"

// テクスチャなし・定数色＋エミッシブのみを返す最小PS
float4 main(VSOutput i) : SV_Target
{
    float3 color = saturate(BaseColor.rgb + Emissive);
    return float4(color, BaseColor.a * Opacity);
}