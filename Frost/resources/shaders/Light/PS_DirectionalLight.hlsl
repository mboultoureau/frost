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

// Returns true if the world position is within this cascade's view volume
bool IsInCascadeView(float3 worldPos, out float2 shadowUV, out float depth)
{
    float4 wp = float4(worldPos, 1.0f);
    float4 sp = mul(wp, LightViewProj);
    sp.xyz /= sp.w;
    
    // Convert to UV coordinates
    shadowUV = sp.xy * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;
    depth = sp.z;
    
    // Check if within bounds (with small epsilon to avoid edge artifacts)
    const float epsilon = 0.001f;
    bool inBounds = (shadowUV.x >= epsilon) && (shadowUV.x <= 1.0f - epsilon) &&
                    (shadowUV.y >= epsilon) && (shadowUV.y <= 1.0f - epsilon) &&
                    (depth >= 0.0f) && (depth <= 1.0f);
    
    return inBounds;
}

// Sample shadow with 3x3 PCF
float SampleCascadedShadowPCF(float3 worldPos, float3 normal, float3 lightPos, float3 lightDir, uint shadowResolution, float2 baseUV, float baseDepth)
{
    float texel = 1.0f / (float) shadowResolution;
    float sum = 0.0f;
    float bias = ComputeShadowBias(worldPos, normal, lightPos, lightDir, shadowResolution);
    
    [unroll]
    for (int dx = -1; dx <= 1; ++dx)
    {
        [unroll]
        for (int dy = -1; dy <= 1; ++dy)
        {
            float2 sampleUV = baseUV + float2(dx, dy) * texel;
            float depth = ShadowMap.Sample(GBufferSampler, sampleUV).r;
            if (depth + bias > baseDepth)
                sum += 1.0f;
        }
    }
    return sum / 9.0f;
}

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