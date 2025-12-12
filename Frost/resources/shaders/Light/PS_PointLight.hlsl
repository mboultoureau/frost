#include "LightingCommon.hlsli"

cbuffer LightConstants : register(b0)
{
    float3 CameraPosition;
    uint ShadowResolution;
    float3 Position;
    float Radius;
    float3 Color;
    float Intensity;
    float3 LightDirection;
};

float4 main(PS_Input input) : SV_TARGET
{
    // 1. Accumulation
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    
    
    float dist = distance(Position, worldPos);
    if (dist >= Radius)
    {
        return float4(currentLight, 1.0f);
    }

    float2 shadowUV;
    float shadowDepth;
    if (!IsInsideShadowFrustum(worldPos, shadowUV, shadowDepth))
    {
        return float4(currentLight, 1.0f);
    }

    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb * 2.0f - 1.0f);
    float4 mat = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float metal = mat.r;
    float rough = mat.g;
    
    float3 lightDir = normalize(Position - worldPos);
    
    float shadowFactor = SampleShadowPCF_Merged(shadowUV, shadowDepth, normal, lightDir, ShadowResolution);
    
    if (shadowFactor <= 0.001f)
    {
        return float4(currentLight, 1.0f);
    }

    // Attenuation
    float atten = saturate(1.0f - (dist / Radius));
    atten *= atten;
    
    float3 viewDir = normalize(CameraPosition - worldPos);
    float3 lightCol = Color * Intensity * atten;

    currentLight += CalculateBlinnPhong(lightDir, viewDir, normal, lightCol, metal, rough) * shadowFactor;

    return float4(currentLight, 1.0f);
}