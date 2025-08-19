#include "Shader.hlsli"

VSOutput main(VSInput i)
{
    VSOutput o;

    float4 localPos = float4(i.position, 1.0);
    float4 worldPos4 = mul(localPos, World);
    float4 viewPos = mul(worldPos4, View);
    float4 projPos = mul(viewPos, Projection);

    o.position = projPos; // �� �o�͂��� SV_Position
    o.Nrm = mul(float4(i.normal, 0), World).xyz; // �������胏�[���h�@���i�񐳋K���Ȃ�normalize�j
    o.Tex = i.tex;
    o.worldPos = worldPos4.xyz; // PS�Ŏ����x�N�g���v�Z�Ɏg��

    return o;
}
