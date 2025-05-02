cbuffer PolyConstantBuffer
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(float3 inPos : POSITION, float4 inColor : COLOR)
{
    VSOutput outV;

    float4 worldPosition = mul(float4(inPos, 1.0f), World);
    float4 viewPosition = mul(worldPosition, View);
    float4 projPosition = mul(viewPosition, Projection);

    outV.position = projPosition;
    outV.color = inColor;

    return outV;
}
