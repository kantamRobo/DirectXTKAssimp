
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