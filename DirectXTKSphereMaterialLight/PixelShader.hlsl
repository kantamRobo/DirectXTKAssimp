#include "Shader.hlsli"

static const float PI = 3.14159265359;

float pow5(float x)
{
    float x2 = x * x;
    return x2 * x2 * x;
}

float3 fresnelSchlick(float u, float3 F0)
{
    return F0 + (1.0 - F0) * pow5(1.0 - u);
}

float D_GTR2(float NdotH, float a)
{
    float a2 = a * a;
    float t = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * t * t);
}

float G1_SmithGGX(float NdotX, float a)
{
    float a2 = a * a;
    float t = NdotX + sqrt(a2 + (1.0 - a2) * NdotX * NdotX);
    return 2.0 * NdotX / max(t, 1e-5);
}

float disneyDiffuseTerm(float NdotL, float NdotV, float LdotH, float roughness, float subsurface)
{
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    float FL = 1.0 + (Fd90 - 1.0) * pow5(1.0 - NdotL);
    float FV = 1.0 + (Fd90 - 1.0) * pow5(1.0 - NdotV);
    float fd = (FL * FV) * (1.0 / PI);

    float Fss90 = Fd90;
    float FLss = 1.0 + (Fss90 - 1.0) * pow5(1.0 - NdotL);
    float FVss = 1.0 + (Fss90 - 1.0) * pow5(1.0 - NdotV);
    float ss = 1.25 * ((FLss * FVss) * (1.0 / max(NdotL + NdotV, 1e-5) - 0.5) + 0.5);

    return lerp(fd, ss * (1.0 / PI), saturate(subsurface));
}

float3 computeTint(float3 c)
{
    float lum = dot(c, float3(0.299, 0.587, 0.114));
    return (lum > 0.0) ? (c / lum) : float3(1.0, 1.0, 1.0);
}

float4 main(VSOutput i) : SV_Target
{
    float3 N = normalize(i.Nrm);
    float3 L = normalize(-ligDirection.xyz);
    float3 V = normalize(eyePos.xyz - i.worldPos);
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float LdotH = saturate(dot(L, H));
    float VdotH = saturate(dot(V, H));

    float3 baseColor = saturate(BaseColor.rgb);
    float metallic = saturate(Metallic);
    float roughness = saturate(Roughness);
    float opacity = saturate(Opacity);

    float specular = saturate(Specular);
    float specularTint = saturate(SpecularTint);
    float sheen = saturate(Sheen);
    float sheenTint = saturate(SheenTint);
    float clearcoat = saturate(Clearcoat);
    float clearcoatGloss = saturate(ClearcoatGloss);
    float subsurface = saturate(Subsurface);

    float alpha = max(roughness * roughness, 0.001);

    float3 ctint = computeTint(baseColor);
    float3 F0_dielectric = 0.08 * specular * lerp(float3(1, 1, 1), ctint, specularTint);
    float3 F0 = lerp(F0_dielectric, baseColor, metallic);

    float fdTerm = disneyDiffuseTerm(NdotL, NdotV, LdotH, roughness, subsurface);
    float3 diffuse = (1.0 - metallic) * baseColor * fdTerm;

    float D = D_GTR2(NdotH, alpha);
    float G = G1_SmithGGX(NdotL, alpha) * G1_SmithGGX(NdotV, alpha);
    float3 F = fresnelSchlick(VdotH, F0);
    float3 specularBRDF = (D * G) * F / max(4.0 * NdotL * NdotV, 1e-5);

    float3 sheenColor = lerp(float3(1, 1, 1), ctint, sheenTint);
    float3 sheenTerm = sheen * sheenColor * pow5(1.0 - LdotH);

    float alphaCoat = lerp(0.1, 0.001, clearcoatGloss);
    float Dc = D_GTR2(NdotH, alphaCoat);
    float Gc = G1_SmithGGX(NdotL, 0.25) * G1_SmithGGX(NdotV, 0.25);
    float Fc = lerp(0.04, 1.0, pow5(1.0 - VdotH));
    float coatBRDF = clearcoat * 0.25 * (Dc * Gc * Fc) / max(4.0 * NdotL * NdotV, 1e-5);

    float3 radiance = ligColor.rgb;
    float3 color = (diffuse + specularBRDF + sheenTerm + coatBRDF) * radiance * NdotL + Emissive;

    return float4(saturate(color), BaseColor.a * opacity);
}
