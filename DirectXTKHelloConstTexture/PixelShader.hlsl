// ── PixelShader.hlsl ──────────────────────
#include "Shader.hlsli"

float4 main(VSOutput input) : SV_TARGET // エントリ名は "main" に統一
{
    return input.color;
}
