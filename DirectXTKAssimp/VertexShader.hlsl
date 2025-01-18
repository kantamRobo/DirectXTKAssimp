#include "Model.hlsli"
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    /*
    // スキニング計算
    float4 skinnedPosition = float4(0.0, 0.0, 0.0, 0.0);
    float3 skinnedNormal = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 4; i++) // 最大4つのボーンをサポート
    {
        if (input.boneWeights[i] > 0.0f)
        {
            float4x4 boneTransform = BoneTransforms[input.boneIndices[i]];
            skinnedPosition += mul(input.pos, boneTransform) * input.boneWeights[i];
            skinnedNormal += mul(float4(input.normal, 0.0f), boneTransform).xyz * input.boneWeights[i];
        }
    }
    */
    // ワールド変換、ビュー変換、プロジェクション変換を適用
    float4 worldPosition = mul(input.pos, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // 法線をワールド座標系に変換
    //output.normal = normalize(mul(float4(skinnedNormal, 0.0f), World).xyz);
	output.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
    // テクスチャ座標はそのまま出力
    output.tex = input.tex;

    return output;
}
