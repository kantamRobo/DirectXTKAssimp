Texture2D g_pathTraceOutput : register(t0);
SamplerState g_linearSampler : register(s0);

float3 ACESFilm(float3 x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    float3 hdr = g_pathTraceOutput.Sample(g_linearSampler, uv).rgb;
    float3 ldr = ACESFilm(hdr);
    ldr = pow(ldr, 1.0f / 2.2f);
    return float4(ldr, 1.0f);
}
