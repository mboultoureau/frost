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
    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb * 2.0f - 1.0f);
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float metalness = material.r;
    float roughness = material.g;
    
    // Calculate distance from camera to pixel
    float3 viewVector = worldPos - CameraPosition;
    float pixelDepth = length(viewVector);
    
    // Check if this pixel is within this cascade's depth range
    bool inCascadeDepthRange = (pixelDepth >= CascadeNear) && (pixelDepth <= CascadeFar);
    
    if (inCascadeDepthRange)
    {
        // Check if within shadow map bounds
        float2 shadowUV;
        float depth;

        bool isInCascad = IsInCascadeView(worldPos, shadowUV, depth);
//        float shadowFactor = SampleShadowPCF(worldPos, ShadowResolution);
        float shadowFactor = SampleCascadedShadowPCF(worldPos, normal, LightPosition, Direction, ShadowResolution, shadowUV, depth);
        float3 viewDir = normalize(CameraPosition - worldPos);
        float3 lightDir = -normalize(Direction);
        float3 lightColor = Color * Intensity;
        currentLight += CalculateBlinnPhong(lightDir, viewDir, normal, lightColor, metalness, roughness) * shadowFactor;
        
    }
    
    return float4(currentLight, 1.0f);
}