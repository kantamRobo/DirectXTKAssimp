#include "Model.hlsli"

// サンプラーステートとテクスチャリソース
Texture2D tex : register(t0);           // テクスチャリソース (シェーダーリソースビューからバインド)
SamplerState texSampler : register(s0); // サンプラーステート (サンプリング方法を制御)

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // テクスチャ色
    float4 texColor = tex.Sample(texSampler, input.tex);

    // ライト方向
    float3 toLight = lightPos - input.worldPos;
    float distance = length(toLight);
    toLight = normalize(toLight);

    // 減衰計算
    float att = 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);

    // Lambert反射
    float NdotL = max(dot(normalize(input.normal), toLight), 0.0f);

    // Diffuse計算
    float3 diffuse = texColor.rgb * lightColor * NdotL * intensity * att;

    // 出力
    return float4(diffuse, texColor.a);
}
