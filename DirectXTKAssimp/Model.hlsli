

// 頂点シェーダーの出力
struct PSInput
{
    float4 position : SV_POSITION; // クリップ空間の位置
    float4 color : COLOR; // 頂点カラー
    float2 texcoord : TEXCOORD; // テクスチャ座標
};

struct VS_INPUT
{
    float4 pos : POSITION; // 頂点位置
    float3 normal : NORMAL; // 法線
    float2 tex : TEXCOORD0; // テクスチャ座標
   // float4 boneWeights : BLENDWEIGHT; // ボーンのウェイト
   // uint4 boneIndices : BLENDINDICES; // ボーンのインデックス
};

cbuffer ConstantBuffer
{
    float4x4 World; // ワールド変換行列
    float4x4 View; // ビュー変換行列
    float4x4 Projection; // 透視射影変換行列
};

cbuffer BoneBuffer
{
    float4x4 BoneTransforms[64]; // ボーン変換行列（最大64ボーンを仮定）
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION; // 出力のスクリーン座標
    float3 normal : NORMAL; // 出力の法線
    float2 tex : TEXCOORD0; // 出力のテクスチャ座標
};
