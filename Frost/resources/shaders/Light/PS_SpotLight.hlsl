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
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;
   
    // 1. Distance check
    float3 toLight = Position - worldPos;
    float dist = length(toLight);

    if (dist >= Radius)
    {
        return float4(currentLight, 1.0f);
    }

    // 2. Spot Angle Check
    float3 lightDir = normalize(toLight);
    // Le dot product renvoie 1.0 si aligné, < 1.0 si écarté.
    // OuterConeAngle est le cosinus de l'angle max.
    float spotAngle = dot(lightDir, -normalize(Direction));

    if (spotAngle <= OuterConeAngle)
    {
        return float4(currentLight, 1.0f);
    }

    // 3. Shadow Frustum Check (Optimisation)
    // Vérifie si le pixel est vu par la caméra d'ombre du Spot
    float2 shadowUV;
    float shadowDepth;
    
    if (!IsInsideShadowFrustum(worldPos, shadowUV, shadowDepth))
    {
        // Si on est dans le cone théorique mais hors de la shadow map (ex: derrière le plan near), on n'éclaire pas
        return float4(currentLight, 1.0f);
    }

    // 4. Calculs finaux (seulement si nécessaire)
    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb);
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float metalness = material.r;
    float roughness = material.g;

    float attenuation = 1.0 - (dist / Radius);
    attenuation *= attenuation;

    float coneFalloff = smoothstep(OuterConeAngle, InnerConeAngle, spotAngle);
    attenuation *= coneFalloff;

    // 5. Shadow Sampling Corrigé
    // On utilise SampleShadowPCF_Merged (au lieu de Optimized) 
    // et on passe 'normal' et 'lightDir' pour le calcul du Slope-Scaled Bias.
    float shadowFactor = SampleShadowPCF_Merged(shadowUV, shadowDepth, normal, lightDir, shadowResolution);
    
    if (shadowFactor > 0.001f)
    {
        float3 viewDir = normalize(CameraPosition - worldPos);
        float3 lightColor = Color * Intensity * attenuation;
        currentLight += CalculateBlinnPhong(lightDir, viewDir, normal, lightColor, metalness, roughness) * shadowFactor;
    }
    
    return float4(currentLight, 1.0f);
}