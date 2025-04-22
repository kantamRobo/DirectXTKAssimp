// 構造体の定義
struct RWStructuredBuffer
{
    uint index;
    float value;
};

// 読み書きバッファ
RWStructuredBuffer<MyData> bufferData : register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    // インデックスに対応する要素を変更
    bufferData[DTid.x].value += 1.0f;
}
