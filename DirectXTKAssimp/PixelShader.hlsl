#include "Model.hlsli"
// サンプラーステートとシェーダーリソースビュー (ピクセルシェーダー用)
Texture2D tex : register(t0);        // テクスチャリソース
SamplerState texSampler : register(s0); // サンプラー


float4 main(PSInput input) : SV_TARGET{
    // テクスチャをサンプリングし、頂点カラーと乗算
    float4 texColor = tex.Sample(texSampler, input.texcoord);
    return texColor * input.color;
}