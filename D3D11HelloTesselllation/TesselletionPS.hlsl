// Tessellation.hlsl
/*
cbuffer TessFactorBuffer : register(b0)
{
    float Inner;    // SV_InsideTessFactor に対応
    float Outer;    // SV_TessFactor に対応（3 要素すべてに同じ値を流す想定）
};

// VS: 単に入力頂点を次へ渡す
struct VSOut
{
    float3 Pos : POSITION;
};
VSOut VSMain(uint vid : SV_VertexID)
{
    VSOut o;
    // 頂点バッファから自動取得
    o.Pos = float3(0, 0, 0);
    // SV_VertexID で頂点バッファを indexing する場合は
    // StructuredBuffer 等を使う必要がありますが、
    // 本サンプルでは IASetVertexBuffers で渡す単純バッファを使用します。
    // HLSL では頂点バッファが直接見えないため、この関数では使いません。
    return o;
}

// HS: 3 control-point patch を受け取り、出力制御点とパッチ定数を生成
struct HSOut
{
    float3 Pos : POSITION;
};

struct HSConst
{
    float EdgeTess[3]     : SV_TessFactor;       // 外周
    float InsideTess : SV_InsideTessFactor; // 内部
};

// DS: バリセンタリック補間
struct DSOut
{
    float4 Pos : SV_POSITION;
};

// PS: 単色
float4 PSMain(DSOut i) : SV_Target
{
    // 座標を色にマッピング
    float3 c = i.Pos.xyz * 0.5f + 0.5f;
    return float4(c, 1.0f);
}
*/
/*
// DS: バリセンタリック補間
struct DSOut
{
    float4 Pos : SV_POSITION;
};

// TesselletionPS.hlsl
float4 PSMain(DSOut i) : SV_Target
{
    return float4(1,0,0,1);  // 真っ赤
}

//「VS→HS→DS→PS→RS→OM」の流れは正しく動いていることが確定
*/

// TesselletionPS.hlsl
struct DSOut
{
    float4 Pos : SV_POSITION;
};

float4 PSMain(DSOut i) : SV_Target
{
    // NDC座標（-1～+1）を 0～1 に正規化
    float3 c = i.Pos.xyz * 0.5f + 0.5f;
    return float4(c, 1.0f);
}
