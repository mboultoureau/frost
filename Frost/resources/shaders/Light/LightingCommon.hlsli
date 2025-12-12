// Lighting Pass: Pixel Shader
Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D WorldPosTexture : register(t2);
Texture2D MaterialTexture : register(t3); // R: Metalness, G: Roughness, B: AO
Texture2D ShadowMap : register(t4);
Texture2D LuminanceTexture : register(t5);
SamplerState GBufferSampler : register(s0);

cbuffer ShadowMatrix : register(b1)
{
    column_major float4x4 World;
    column_major float4x4 LightViewProj;
}

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};


float ComputeShadowBias(float3 normal, float3 lightDir, float shadowResolution)
{
    float ndotl = saturate(dot(normal, lightDir));
    float texelSize = 1.0f / shadowResolution;
    
    float slope = sqrt(1.0f - ndotl * ndotl) / max(ndotl, 0.01f);
    
    float constantBias = 0.5f * texelSize;
    float slopeBias = slope * 1.0f * texelSize;
    
    return min(constantBias + slopeBias, 0.01f);
}

bool IsInsideShadowFrustum(float3 worldPos, out float2 uv, out float z)
{
    float4 wp = float4(worldPos, 1.0f);
    float4 sp = mul(wp, LightViewProj);
    sp.xyz /= sp.w;

    uv = sp.xy * 0.5f + 0.5f;
    uv.y = 1.0f - uv.y;
    z = sp.z;

    return (uv.x >= 0.0f && uv.x <= 1.0f &&
            uv.y >= 0.0f && uv.y <= 1.0f &&
            z >= 0.0f && z <= 1.0f);
}

float SampleShadowPCF_Merged(float2 baseUV, float baseDepth, float3 normal, float3 lightDir, uint shadowResolution)
{
    float texel = 1.0f / (float) shadowResolution;
    float sum = 0.0f;
    
    float bias = ComputeShadowBias(normal, lightDir, shadowResolution);
    
    [unroll]
    for (int dx = -1; dx <= 1; ++dx)
    {
        [unroll]
        for (int dy = -1; dy <= 1; ++dy)
        {
            float2 sampleUV = baseUV + float2(dx, dy) * texel;
            float mapDepth = ShadowMap.Sample(GBufferSampler, sampleUV).r;
            
            if (mapDepth + bias > baseDepth)
            {
                sum += 1.0f;
            }
        }
    }
    return sum / 9.0f;
}

float3 CalculateBlinnPhong(float3 lightDir, float3 viewDir, float3 normal, float3 lightColor, float metalness, float roughness)
{
    // Diffuse
    float NdotL = saturate(dot(normal, lightDir));
    float3 diffuse = (1.0 - metalness) * lightColor * NdotL;

    // Specular
    float3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal, halfwayDir));
    
    // Roughness remapping (GGX style approx)
    float alpha = roughness * roughness;
    float specPower = 2.0 / (alpha * alpha) - 2.0;
    specPower = max(specPower, 1.0);

    float3 specular = pow(NdotH, specPower) * lightColor * (1.0 - roughness);

    return diffuse + specular;
}