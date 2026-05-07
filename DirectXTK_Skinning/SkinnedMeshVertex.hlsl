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
    float3 norm : NORMAL;
    float2 texCoord : TEXCOORD0;
};

cbuffer BoneMatrices
{
    float4x4 g_BoneTransforms[64];
}

// 必要に応じて追加
// cbuffer TransformUBO
// {
//     float4x4 g_WVP;
// }

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    float3 totalPosition = 0.0f;
    float3 transformedNormal = 0.0f;

    // スキニング計算
    for (int i = 0; i < 4; i++)
    {
        uint id = input.boneIds[i];
        float weight = input.weights[i];

        // 無効ボーンチェック (uintなので -1 ではなく 0xFFFFFFFF や 255 等で判定)
        if (id == 0xFFFFFFFF || id >= 64)
            continue;

        // 位置の変形: g_BoneTransforms は column-major 想定
        float4 transformedPos = g_BoneTransforms[id] * float4(input.pos, 1.0f);
        totalPosition += transformedPos.xyz * weight;

        // 法線の変形 (回転成分のみを適用)
        float3x3 boneRot = (float3x3)g_BoneTransforms[id]; // または mat3(g_BoneTransforms[id])
        transformedNormal += mul(float3(input.norm), boneRot) * weight;
    }

    // 最終的な View/Projection 変換
    // 実際のプロジェクトでは g_WVP などを cbuffer から取得
    output.pos = mul(float4(totalPosition, 1.0f), g_WVP);
    output.norm = transformedNormal;
    output.texCoord = input.texCoord;

    return output;
}