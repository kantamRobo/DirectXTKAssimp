// 頂点シェーダー: 入力は float3 の位置だけ
struct VSIn
{
    float3 pos : POSITION;
};

struct PSIn
{
    float4 pos : SV_POSITION;
};

PSIn VSMain(VSIn input)
{
    PSIn o;
    // そのままクリップ空間に出力（NDC変換済み座標想定）
    o.pos = float4(input.pos, 1.0f);
    return o;
}
