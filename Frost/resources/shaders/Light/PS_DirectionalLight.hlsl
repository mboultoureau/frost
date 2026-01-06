#include "LightingCommon.hlsli"

cbuffer LightConstants : register(b0)
{
    float3 CameraPosition;
    uint ShadowResolution;
    float3 Direction;
    float Intensity;
    float3 Color;
    float CascadeNear;
    float3 CameraForward;
    float CascadeFar;
    float3 LightPosition;
    float2 Padding;
};

float4 main(PS_Input input) : SV_TARGET
{
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    
    float3 viewVector = worldPos - CameraPosition;
    float dist = length(viewVector);
    
    if (dist < CascadeNear || dist > CascadeFar)
    {
        return float4(currentLight, 1.0f);
    }

    float2 shadowUV;
    float shadowDepth;
    if (!IsInsideShadowFrustum(worldPos, shadowUV, shadowDepth))
    {
        return float4(currentLight, 1.0f);
    }

    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb);
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float metalness = material.r;
    float roughness = material.g;

    float3 lightDirVec = -normalize(Direction);
    float3 biasLightDir = normalize(LightPosition - worldPos);
    
    float shadowFactor = SampleShadowPCF_Merged(shadowUV, shadowDepth, normal, lightDirVec, ShadowResolution);
    
    if (shadowFactor <= 0.001f)
        return float4(currentLight, 1.0f);

    float3 viewDir = normalize(CameraPosition - worldPos);
    float3 lightColor = Color * Intensity;
    
    currentLight += CalculateBlinnPhong(lightDirVec, viewDir, normal, lightColor, metalness, roughness) * shadowFactor;
    
    return float4(currentLight, 1.0f);
}