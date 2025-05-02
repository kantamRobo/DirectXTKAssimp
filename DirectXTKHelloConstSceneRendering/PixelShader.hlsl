// ── PixelShader.hlsl ──────────────────────
#include "Shader.hlsli"
struct VSOutput        // ← “PSInput” ではなく用途が分かる名前に
{
    float4 position : SV_POSITION; // ★必須
    float4 color : COLOR;
};

float4 main(VSOutput input) : SV_TARGET   // エントリ名は "main" に統一
{
    return input.color;
}
