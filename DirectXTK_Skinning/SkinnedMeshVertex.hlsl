struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 texCoord : TEXCOORD0;
    uint4 boneIds : TEXCOORD1;
    float4 weights : TEXCOORD2;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 norm : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
};

cbuffer BoneMatrices
{
    float4x4 g_BoneTransforms[64];
};

// View/Projection 行列を定義
cbuffer TransformUBO
{
    float4x4 g_WVP;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    float3 totalPosition = 0.0f;
    float3 transformedNormal = 0.0f;
    float totalWeight = 0.0f;

    // スキニング計算
    for (int i = 0; i < 4; i++)
    {
        uint id = input.boneIds[i];
        float weight = input.weights[i];

        // 無効ボーンチェック (uint なので -1 ではなく 0xFFFFFFFF や 255 などで判定)
        if (id == 0xFFFFFFFF || id >= 64)
            continue;

        // 位置の変形: g_BoneTransforms が column-major 配列
               // 修正：mul を使って行ベクトル × 行列 の順に統一
        float4 transformedPos = mul(float4(input.pos, 1.0f), g_BoneTransforms[id]);
        totalPosition += transformedPos.xyz * weight;

        // 法線の変形 (回転成分のみを適用)
        float3x3 boneRot = (float3x3)g_BoneTransforms[id]; // または mat3(g_BoneTransforms[id])
        transformedNormal += mul(float3(input.norm), boneRot) * weight;
        
        totalWeight += weight;
    }

    // ウェイトが0の場合の対応
    if (totalWeight > 0.0001f)
    {
        totalPosition /= totalWeight;
        transformedNormal /= totalWeight;
    }
    else
    {
        // フォールバック：元の値を使用
        totalPosition = input.pos;
        transformedNormal = input.norm;
    }

    // 最終的な View/Projection 変換
    output.pos = mul(float4(totalPosition, 1.0f), g_WVP);
    output.norm = normalize(transformedNormal);
    output.texCoord = input.texCoord;

    return output;
}
