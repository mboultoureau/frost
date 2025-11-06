cbuffer Constants
{
    float4x4 WorldViewProjection;
    float4x4 World;
    
    float4 LightPosition;
    float4 AmbientColor;
    float4 DiffuseColor;
    
    float4 EmissiveColor;
    
    float4 CameraPosition;
    
    int numberDiffuseTextures;
    int hasNormalMap;
    int hasEmissiveTexture;
    int hasAmbientOclusionTexture;
    
    int hasMetallicTexture;
    float roughnessValue;
    int hasRoughnessTexture;
    int padding[1];
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D emissiveTexture : register(t2);
Texture2D ambientOclusionTexture : register(t3);
Texture2D metallicTexture : register(t4);
Texture2D roughnessTexture : register(t5);

SamplerState textureSampler : register(s0);

struct PixelInput
{
    float4 position : SV_Position;
    float3 lightDirection : TEXCOORD0;
    float3 cameraDirection : TEXCOORD1;
    float2 texCoord : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
    float3 worldNormal : TEXCOORD5;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    float4 BaseColor = DiffuseColor;
    if (numberDiffuseTextures > 0)
    {
        BaseColor *= diffuseTexture.Sample(textureSampler, input.texCoord);
    }
    
    float3 finalLighting;
    
    float3 L = normalize(input.lightDirection);
    float3 V = normalize(input.cameraDirection);
    
    
    float3 N;
        
    if (hasNormalMap > 0)
    {
        float3 tangentNormal = normalTexture.Sample(textureSampler, input.texCoord).xyz;
        tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
            
        float3 T = normalize(input.worldTangent);
        float3 B = normalize(input.worldBitangent);
        float3 WorldN = normalize(input.worldNormal);
            
        float3x3 TBN = float3x3(T, B, WorldN);
            
        N = normalize(mul(tangentNormal, TBN));
    }
    else
    {
        N = normalize(input.worldNormal);
    }
    
    float3 diff = saturate(dot(N, L));
    float3 R = normalize(2 * diff.xyz * N - L);
    

    float aoFactor = 1.0f;
    if (hasAmbientOclusionTexture > 0)
    {
        aoFactor = ambientOclusionTexture.Sample(textureSampler, input.texCoord).r;
    }

    float metallicFactor = 0.0f;
    if (hasMetallicTexture > 0)
    {
        metallicFactor = metallicTexture.Sample(textureSampler, input.texCoord).b;
    }
    
    float roughness = roughnessValue;
    if (hasRoughnessTexture > 0)
    {
        roughness = roughnessTexture.Sample(textureSampler, input.texCoord).g;
    }
    
    float shininess = (1.0f - roughness) * 31.0f + 1.0f;
    
    float3 S = pow(saturate(dot(R, V)), shininess); 
    
    float3 BaseDiffuse = BaseColor.rgb * (1.0f - metallicFactor);
    
    float3 nonSpecular = (AmbientColor.rgb * BaseDiffuse) + (BaseDiffuse * diff);

    nonSpecular *= aoFactor;
    
    finalLighting = nonSpecular + S;
 
    
    float3 finalEmissive = EmissiveColor.rgb;
    
    if (hasEmissiveTexture > 0)
    {
        finalEmissive *= emissiveTexture.Sample(textureSampler, input.texCoord).rgb;
    }
    
    float3 finalColor = finalLighting + finalEmissive;
    
    return float4(finalColor, BaseColor.a);
}