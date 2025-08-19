// ── PixelShader.hlsl ──────────────────────
#include "Shader.hlsli"

// テクスチャなし・定数色＋エミッシブのみを返す最小PS
float4 main(VSOutput i) : SV_Target
{
    
    // ピクセルの法線とライトの方向の内積を計算する
    float t = dot(i.Nrm, ligDirection);
    t *= -1.0f;

    // 内積の結果が0以下なら0にする
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    // 拡散反射光を求める
    float3 diffuseLig = ligColor * t;

    // step-4 反射ベクトルを求める
    float3 refVec = reflect(ligDirection, i.Nrm);

    // step-5 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = eyePos - i.worldPos;
    toEye = normalize(toEye);

    // step-6 鏡面反射の強さを求める
    t = dot(refVec, toEye);
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    // step-7 鏡面反射の強さを絞る
    t = pow(t, 5.0f);

    // step-8 鏡面反射光を求める
    float3 specularLig = ligColor * t;

    // step-9 拡散反射光と鏡面反射光を足し算して、最終的な光を求める
    float3 lig = diffuseLig + specularLig;
    float4 finalColor = g_texture.Sample(g_sampler, i.Tex);

    // step-10 テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= lig;
    float3 color = saturate(BaseColor.rgb + Emissive);

    
    return float4(color, BaseColor.a * Opacity);

}

 