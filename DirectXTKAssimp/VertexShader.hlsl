struct VS_INPUT
{
    float4 pos : POSITION;       // 頂点位置
    float3 normal : NORMAL;      // 法線
    float2 tex : TEXCOORD0;      // テクスチャ座標
    float4 boneWeights : BLENDWEIGHT; // ボーンのウェイト
    uint4 boneIndices : BLENDINDICES; // ボーンのインデックス
};

cbuffer ConstantBuffer
{
    float4x4 World;              // ワールド変換行列
    float4x4 View;               // ビュー変換行列
    float4x4 Projection;         // 透視射影変換行列
};

cbuffer BoneBuffer
{
    float4x4 BoneTransforms[64]; // ボーン変換行列（最大64ボーンを仮定）
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;    // 出力のスクリーン座標
    float3 normal : NORMAL;      // 出力の法線
    float2 tex : TEXCOORD0;      // 出力のテクスチャ座標
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

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

    // ワールド変換、ビュー変換、プロジェクション変換を適用
    float4 worldPosition = mul(skinnedPosition, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // 法線をワールド座標系に変換
    output.normal = normalize(mul(float4(skinnedNormal, 0.0f), World).xyz);

    // テクスチャ座標はそのまま出力
    output.tex = input.tex;

    return output;
}
