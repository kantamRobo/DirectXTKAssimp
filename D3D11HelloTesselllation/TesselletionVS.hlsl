// 入力レイアウトに合わせて POSITION を受け取る
struct VSIn
{
    float3 Pos : POSITION;
};
struct VSOut
{
    float3 Pos : POSITION;
};
VSOut VSMain(VSIn vin)
{
    VSOut o;
    // ここで IASetVertexBuffers で渡されたバッファの座標をそのまま使う
    o.Pos = vin.Pos;
    return o;
}