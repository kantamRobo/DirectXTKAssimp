// Tessellation.hlsl

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

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSConstFunc")]
HSOut HSMain(InputPatch<VSOut, 3> patch, uint cid : SV_OutputControlPointID)
{
    HSOut o;
    o.Pos = patch[cid].Pos;
    return o;
}

HSConst HSConstFunc(InputPatch<VSOut, 3> patch)
{
    HSConst o;
    o.EdgeTess[0] = Outer;
    o.EdgeTess[1] = Outer;
    o.EdgeTess[2] = Outer;
    o.InsideTess = Inner;
    return o;
}

// DS: バリセンタリック補間
struct DSOut
{
    float4 Pos : SV_POSITION;
};
[domain("tri")]
DSOut DSMain(HSConst hc, const OutputPatch<HSOut, 3> patch, float3 bary : SV_DomainLocation)
{
    DSOut o;
    float3 p = patch[0].Pos * bary.x
        + patch[1].Pos * bary.y
        + patch[2].Pos * bary.z;
    // そのままクリップ空間と仮定（射影行列などは省略）
    o.Pos = float4(p, 1.0f);
    return o;
}

// PS: 単色
float4 PSMain(DSOut i) : SV_Target
{
    // 座標を色にマッピング
    float3 c = i.Pos.xyz * 0.5f + 0.5f;
    return float4(c, 1.0f);
}
