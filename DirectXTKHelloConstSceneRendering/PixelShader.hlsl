
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
float4 main(VSOutput input) : SV_TARGET   // ƒGƒ“ƒgƒŠ–¼‚Í "main" ‚É“ˆê
{
    return input.color;
}
