cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 padding = 15;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
