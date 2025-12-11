#include "LightingCommon.hlsli"

cbuffer LightConstants : register(b0)
{
    float3 CameraPosition;
    float Intensity;
    float3 Color;
};


float4 main(PS_Input input) : SV_TARGET
{
    float3 albedo = AlbedoTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;

    float ao = material.b;
    
    float3 lightColor = Color * Intensity;
    float3 ambient = lightColor * ao;
    currentLight += ambient;

    
    return float4(currentLight, 1.0f);
}