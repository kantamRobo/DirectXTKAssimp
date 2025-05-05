// ピクセルシェーダー: 赤一色出力
float4 PSMain() : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
