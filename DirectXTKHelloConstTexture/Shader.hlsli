// ── Shader.hlsli ──────────────────────────
cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
};

struct VSOutput        // ← “PSInput” ではなく用途が分かる名前に
{
    float4 position : SV_POSITION; // ★必須
    float4 color : COLOR;
    float2 texcoord : TEXCOORD0;
};
