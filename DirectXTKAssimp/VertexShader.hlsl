#include "Model.hlsli"

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // ワールド変換、ビュー変換、プロジェクション変換を適用
    float4 worldPosition = mul(input.pos, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // 法線とテクスチャ座標はそのまま出力
    output.normal = mul(float4(input.normal, 0.0f), World).xyz; // 法線もワールド座標系に変換
    output.tex = input.tex;
    output.worldPos = worldPosition.xyz;

    return output;
}