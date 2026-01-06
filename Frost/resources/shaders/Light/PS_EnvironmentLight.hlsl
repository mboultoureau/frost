#include "LightingCommon.hlsli"

SamplerState LinearSampler : register(s1);
TextureCube EnvironmentTexture : register(t6);

cbuffer LightConstants : register(b0)
{
    float3 CameraPosition;
    float Intensity;
};

static const float PI = 3.14159265359;
static const float MAX_REFLECTION_LOD = 8.0;

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float4 main(PS_Input input) : SV_TARGET
{
    float4 normalData = NormalTexture.Sample(GBufferSampler, input.TexCoord);
    if (normalData.a < 0.5f)
    {
        float3 existingColor = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;
        return float4(existingColor, 1.0f);
    }
    
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb);
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float3 albedo = AlbedoTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;

    float metalness = material.r;
    float roughness = material.g;
    float ao = material.b;

    float3 V = normalize(CameraPosition - worldPos);
    float3 N = normal;
    float3 R = reflect(-V, N);

    float3 F0 = float3(0.04, 0.04, 0.04); 
    F0 = lerp(F0, albedo, metalness);

    float3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    float3 prefilteredColor = EnvironmentTexture.SampleLevel(LinearSampler, R, roughness * MAX_REFLECTION_LOD).rgb;

    float3 specular = prefilteredColor * F;
    specular *= ao; 

    currentLight += specular * Intensity;

    return float4(currentLight, 1.0f);
}