struct VS_INPUT
{
    /*
    float4 pos : POSITION; // 頂点位置
    float3 normal : NORMAL; // 法線
    float2 tex : TEXCOORD0; // テクスチャ座標
*/
    
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1; // 追加
};
 
cbuffer ConstantBuffer
{
    float4x4 World; // ワールド変換行列
    float4x4 View; // ビュー変換行列
    float4x4 Projection; // 透視射影変換行列
};
cbuffer PointLightBuffer : register(b1)
{
    float3 lightPos; // ポイントライトの位置（ワールド座標系）
    float intensity; // 光の強さ

    float3 lightColor; // 光の色
    float range; // 有効範囲

    float3 attenuation; // 減衰パラメータ
    float padding; // パディング
};
struct VS_OUTPUT
{
    
    /*
    float4 pos : SV_POSITION; // 出力のスクリーン座標
    float3 normal : NORMAL; // 出力の法線
    float2 tex : TEXCOORD0; // 出力のテクスチャ座標

*/
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1; // 追加
};