
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
float4 main(VSOutput input) : SV_TARGET   // エントリ名は "main" に統一
{
    return input.color;
}
