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

// World/View/Projection 行列
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

    // スキニング計算: 各ボーンの影響を積算
    for (int i = 0; i < 4; i++)
    {
        uint boneId = input.boneIds[i];
        float weight = input.weights[i];

        // ウェイトがほぼ0なら影響を無視
        if (weight < 0.0001f)
            continue;

        // ボーン ID の妥当性チェック (0..63 の範囲内か)
        if (boneId >= 64u)
            continue;

        // ボーン行列の取得（C++ 側で転置して渡されている想定）
        float4x4 boneMat = g_BoneTransforms[boneId];

        // 位置の変形: ボーン行列を適用
        // mul(vector, matrix) は行ベクトル x 行列 で計算される
        float4 positionBoneSpace = mul(float4(input.pos, 1.0f), boneMat);
        totalPosition += positionBoneSpace.xyz * weight;

        // 法線の変形: 回転成分のみを適用（スケール・平行移動は除外）
        // 4x4 行列の左上 3x3 が回転・スケール成分
        float3x3 rotationMat = (float3x3)boneMat;
        float3 normalBoneSpace = mul(input.norm, rotationMat);
        transformedNormal += normalBoneSpace * weight;

        // 重みの合計を追跡
        totalWeight += weight;
    }

    // 重み正規化: 各頂点に複数ボーンが影響する場合、重みの合計が1でない可能性がある
    if (totalWeight > 0.0001f)
    {
        totalPosition /= totalWeight;
        transformedNormal /= totalWeight;
    }
    else
    {
        // フォールバック: ウェイトが0に近い場合は元の値を使用
        totalPosition = input.pos;
        transformedNormal = input.norm;
    }

    // 最終的な World/View/Projection 変換
    output.pos = mul(float4(totalPosition, 1.0f), g_WVP);
    output.norm = normalize(transformedNormal);
    output.texCoord = input.texCoord;

    return output;
}