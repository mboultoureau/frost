// Lighting Pass: Pixel Shader
Texture2D AlbedoTexture         : register(t0); // RGB: Albedo
Texture2D NormalTexture         : register(t1); // RGB: Normal
Texture2D WorldPosTexture       : register(t2); // RGB: World Position
Texture2D MaterialTexture       : register(t3); // R: Metalness, G: Roughness, B: Ambient Occlusion

SamplerState GBufferSampler     : register(s0);

#define MAX_DIRECTIONAL_LIGHTS 2
#define MAX_POINT_LIGHTS 100
#define MAX_SPOT_LIGHTS 50

struct DirectionalLight
{
    float3  Direction;
    float   Padding;
    float3  Color;
    float   Intensity;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
    float Intensity;
    float Falloff;
    float3 Padding;
};

struct SpotLight
{
    float3  Position;
    float   Range;
    float3  Direction;
    float   Intensity;
    float3  Color;
    float   InnerConeCos;  // cos(angle)
    float   OuterConeCos;  // cos(angle)
    float3  Padding;
};

cbuffer LightConstants : register(b0)
{
    float3              CameraPosition;
    float               Padding0;
    DirectionalLight    DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight          PointLights[MAX_POINT_LIGHTS];
    SpotLight           SpotLights[MAX_SPOT_LIGHTS];
    int                 NumDirectionalLights;
    int                 NumPointLights;
    int                 NumSpotLights;
    float               Padding1;
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float CalculateAttenuation(float dist, float radius, float falloffExponent)
{
    float d = saturate(1.0f - (dist / radius));
    return pow(d, falloffExponent);
}

float3 CalculateBlinnPhong(float3 lightDir, float3 viewDir, float3 normal, float3 lightColor, float metalness, float roughness)
{
    // Diffuse
    float NdotL = saturate(dot(normal, lightDir));
    float3 diffuse = (1.0 - metalness) * lightColor * NdotL;

    // Specular
    float3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal, halfwayDir));
    
    // Roughness -> Specular power
    float specPower = exp2(10.0 * (1.0 - roughness) + 1.0);
    float3 specular = pow(NdotH, specPower) * lightColor;
    
    // Metalness blends
    specular = lerp(specular, specular * diffuse, metalness);

    return diffuse + specular;
}


float4 main(PS_Input input) : SV_TARGET
{
    float3 albedo = AlbedoTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb * 2.0f - 1.0f);
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float4 material = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    
    float metalness = material.r;
    float roughness = material.g;
    float ao = material.b;

    if (length(normal) < 0.1f)
        discard;

    float3 viewDir = normalize(CameraPosition - worldPos);
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    float3 ambient = float3(0.03f, 0.03f, 0.05f) * albedo * ao;
    finalColor += ambient;

    for (int i = 0; i < NumDirectionalLights; ++i)
    {
        float3 lightDir = -normalize(DirectionalLights[i].Direction);
        float3 lightColor = DirectionalLights[i].Color * DirectionalLights[i].Intensity;
        finalColor += CalculateBlinnPhong(lightDir, viewDir, normal, lightColor, metalness, roughness) * albedo;
    }

    for (i = 0; i < NumPointLights; ++i)
    {
        float3 toLight = PointLights[i].Position - worldPos;
        float distance = length(toLight);

        if (distance < PointLights[i].Radius)
        {
            float3 lightDir = normalize(toLight);
            float attenuation = CalculateAttenuation(distance, PointLights[i].Radius, PointLights[i].Falloff);
            
            float3 lightColor = PointLights[i].Color * PointLights[i].Intensity * attenuation;
            finalColor += CalculateBlinnPhong(lightDir, viewDir, normal, lightColor, metalness, roughness) * albedo;
        }
    }

    for (i = 0; i < NumSpotLights; ++i)
    {
        float3 toLight = SpotLights[i].Position - worldPos;
        float distance = length(toLight);

        if (distance < SpotLights[i].Range)
        {
            float3 lightDir = normalize(toLight);
            
            float spotCos = dot(lightDir, normalize(SpotLights[i].Direction));

            if (spotCos > SpotLights[i].OuterConeCos)
            {
                float distAtten = CalculateAttenuation(distance, SpotLights[i].Range, 2.0f);
                float coneAtten = smoothstep(SpotLights[i].OuterConeCos, SpotLights[i].InnerConeCos, spotCos);
                float3 lightColor = SpotLights[i].Color * SpotLights[i].Intensity * distAtten * coneAtten;
                
                finalColor += CalculateBlinnPhong(lightDir, viewDir, normal, lightColor, metalness, roughness) * albedo;
            }
        }
    }

    return float4(finalColor, 1.0f);
}
