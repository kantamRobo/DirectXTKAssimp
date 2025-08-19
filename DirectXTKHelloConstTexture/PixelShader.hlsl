#include "Shader.hlsli"

float4 main(VSOutput i) : SV_Target
{
    float3 N = normalize(i.Nrm);
    float3 L = normalize(-ligDirection.xyz); // light -> surface
    float3 V = normalize(eyePos.xyz - i.worldPos);
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    float NdotH = saturate(dot(N, H));

    float3 albedo = saturate(BaseColor.rgb);
    float metallic = saturate(Metallic);
    float roughness = saturate(Roughness);
    float opacity = saturate(Opacity);

    // Diffuse term (no diffuse for metals in this simple model)
    float3 kd = (1.0 - metallic);
    float3 diffuse = kd * albedo * NdotL;

    // Simple Blinn-Phong specular driven by roughness
    float shininess = lerp(128.0, 4.0, roughness * roughness);
    float3 specColor = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float specTerm = pow(NdotH, shininess);
    float3 specular = specColor * specTerm * NdotL;

    float3 radiance = ligColor.rgb;
    float3 color = (diffuse + specular) * radiance + Emissive;

    return float4(saturate(color), BaseColor.a * opacity);
}
