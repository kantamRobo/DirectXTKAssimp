

cbuffer PolyConstantBuffer:register(b0)
{
    float4x4 World; // ワールド変換行列
    float4x4 View; // ビュー変換行列
    float4x4 Projection; // 透視射影変換行列
};

struct VSOutput        // ← “PSInput” ではなく用途が分かる名前に
{
    float4 position : SV_Position; // ★必須
    float4 color : COLOR;
    float3 Nrm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

#define Roughness (Params.x)
#define Metallic  (Params.y)
#define Opacity   (Params.z)

// Material.hlsli
cbuffer MaterialCB : register(b1)
{
    float4 BaseColor; // RGBA
    float3 Emissive;
    float _pad0; // 16B境界合わせ
    float4 Params; // x=Roughness, y=Metallic, z=Opacity, w=Unused
}

// ディレクションライト用のデータを受け取るための定数バッファーを用意する
cbuffer DirectionLightCb : register(b2)
{
    float3 ligDirection; //ライトの方向
    float3 ligColor; //ライトのカラー

    // step-3 視点のデータにアクセスするための変数を定数バッファーに追加する
    float3 eyePos; //視点の位置
};


///////////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

///////////////////////////////////////////
// サンプラーステート
///////////////////////////////////////////
sampler g_sampler : register(s0);