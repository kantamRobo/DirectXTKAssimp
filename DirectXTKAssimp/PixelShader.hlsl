#include "Model.hlsli"

// サンプラーステートとテクスチャリソース
Texture2D tex : register(t0);           // テクスチャリソース (シェーダーリソースビューからバインド)
SamplerState texSampler : register(s0); // サンプラーステート (サンプリング方法を制御)

// ピクセルシェーダー
float4 main(VS_OUTPUT input) : SV_TARGET
{
    // テクスチャをサンプリングして色を取得
    float4 texColor = tex.Sample(texSampler, input.tex);

// テクスチャの色をそのまま出力
return texColor;
}