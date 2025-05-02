
// HLSL：register(b0) に変換行列、register(b1) にオフセット
cbuffer TransformCB : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

cbuffer OffsetCB : register(b1)
{
    float4 offset;
    // 必要ならパディング
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(float3 inPos : POSITION, float4 inColor : COLOR)
{
    VSOutput outV;


    //座標変換が上手くバインドされていない(シェーダーのバインド自体は出来ている)
    float4 worldPosition = mul(float4(inPos, 1.0f), World);
    float4 viewPosition = mul(worldPosition, View);
    float4 projPosition = mul(viewPosition, Projection);

    outV.position = projPosition;
    outV.color = inColor;

    return outV;
}
