cbuffer PolyConstantBuffer
{
    float4x4 World; // ワールド変換行列
    float4x4 View; // ビュー変換行列
    float4x4 Projection; // 透視射影変換行列
};
// ── Shader.hlsli ──────────────────────────
cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
};

// ── PixelShader.hlsl 
struct VSOutput        // ← “PSInput” ではなく用途が分かる名前に
{
    float4 position : SV_POSITION; // ★必須
    float4 color : COLOR;
};
