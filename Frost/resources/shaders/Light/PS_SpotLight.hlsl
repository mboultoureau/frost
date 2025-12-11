#include "LightingCommon.hlsli"

cbuffer LightConstants : register(b0)
{
    float3 CameraPosition; 
    float InnerConeAngle; // cos(angle)
    float3 Position;
    float Radius;
    float3 Direction;
    float Intensity;
    float3 Color;
    float OuterConeAngle; // cos(angle)
    uint shadowResolution;
};

float4 main(PS_Input input) : SV_TARGET
{
    //float3 albedo = AlbedoTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb * 2.0f - 1.0f);
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float metalness = material.r;
    float roughness = material.g;
    float ao = material.b;

    float3 viewDir = normalize(CameraPosition - worldPos);
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;
   
    float3 toLight = Position - worldPos;
    float distance = length(toLight);

    if (distance < Radius)
    {
        float3 lightDir = normalize(toLight);
        float spotAngle = dot(lightDir, -normalize(Direction));

        if (spotAngle > OuterConeAngle)
        {
            float attenuation = 1.0 - (distance / Radius);
            attenuation *= attenuation;

            float coneFalloff = smoothstep(OuterConeAngle, InnerConeAngle, spotAngle);
            attenuation *= coneFalloff;

            float3 lightColor = Color * Intensity * attenuation;
            float shadowFactor = SampleShadowPCF(worldPos, shadowResolution);
            currentLight += CalculateBlinnPhong(lightDir, viewDir, normal, lightColor, metalness, roughness) * shadowFactor;
        }
    }
    

    //currentLight = float3(1, 1, 1);
    return float4(currentLight, 1.0f);
}