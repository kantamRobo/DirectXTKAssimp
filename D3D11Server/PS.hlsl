
struct PSOut
{
    float4 color0 : SV_Target0; // rtvWin
    float4 color1 : SV_Target1; // rtvOff
};
PSOut PSMain() : SV_Target
{
    PSOut o;
    o.color0 = float4(1, 0, 0, 1);
    o.color1 = float4(1, 0, 0, 1);
    return o;
}


/*

2. マルチレンダーターゲット対応のピクセルシェーダにする
もし１回のドローコールで両方に同時に出力したいなら、PS をこんな風に書き換えます：

hlsl
コピーする
編集する
struct PSOut {
    float4 color0 : SV_Target0;  // rtvWin
    float4 color1 : SV_Target1;  // rtvOff
};

PSOut PSMain(...) {
    PSOut o;
    o.color0 = float4(1,0,0,1);
    o.color1 = float4(1,0,0,1);
    return o;
}
この場合は従来どおり

cpp
コピーする
編集する
ctx->OMSetRenderTargets(2, rtvs, nullptr);
ctx->Draw(count, 0);
swap->Present(1, 0);
ctx->CopyResource(stageTex, offTex);
とすれば、一度で両方に三角形が描かれます。

以上のいずれかで、オフスクリーン用テクスチャにもちゃんと三角形が描き込まれるようになり、ホスト側の表示も赤い三角形になるはずです。
*/