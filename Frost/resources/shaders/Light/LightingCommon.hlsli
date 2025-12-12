// Lighting Pass: Pixel Shader
Texture2D AlbedoTexture : register(t0); // RGB: Albedo
Texture2D NormalTexture : register(t1); // RGB: Normal
Texture2D WorldPosTexture : register(t2); // RGB: World Position
Texture2D MaterialTexture : register(t3); // R: Metalness, G: Roughness, B: Ambient Occlusion
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

float ComputeShadowBias(float3 worldPos, float3 normal, float3 lightPos, float3 lightDir, float shadowResolution)
{
    lightDir = normalize(-lightDir);
    float texelSize = 1.0f / shadowResolution;
    float ndotl = saturate(dot(normal, lightDir));
    float slope = sqrt(1.0f - ndotl * ndotl) / max(ndotl, 0.01f);
    float constantBias = .5f * texelSize;
    float slopeBias = slope * 1.0f * texelSize;
    return constantBias + slopeBias;
}

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

// Sample shadow with 3x3 PCF using comparison sampler
float SampleShadowPCF(float3 worldPos, uint shadowResolution)
{
    float4 wp = float4(worldPos, 1.0f);
    column_major float4x4 lMat = LightViewProj;
    //column_major float4x4 wMat = World;
    
    float4 sp = mul(wp, lMat);
    sp.xyz /= sp.w;

    float2 uv = sp.xy * 0.5f + 0.5f;
    uv.y = 1 - uv.y;
    float z = sp.z; // depth in light clip (0..1)

    if ((uv.x < 0.0f) || (uv.x > 1.0f) || (uv.y < 0.0f) || (uv.y > 1.0f))
    {
        return .0f;
    }
    else
    {
        float texel = 1.0f / (float) shadowResolution;
        float sum = 0.0f;

        [unroll]
        for (int dx = -1; dx <= 1; ++dx)
        {
            [unroll]
            for (int dy = -1; dy <= 1; ++dy)
            {
                float2 sampleUV = uv + float2(dx, dy) * texel;
                // SampleCmpLevelZero returns 1 if depth <= mapDepth (lit), 0 if in shadow,
                // this uses the comparison sampler (ShadowSampler)
                float depth = ShadowMap.Sample(GBufferSampler, sampleUV).r + 0.00005;
                if (depth > z)
                {
                    sum += 1;
                }
            }
        }

        return sum / 9.0f;
    }
}

float3 CalculateBlinnPhong(float3 lightDir, float3 viewDir, float3 normal, float3 lightColor, float metalness, float roughness)
{
    // Diffuse
    float NdotL = saturate(dot(normal, lightDir));
    float3 diffuse = (1.0 - metalness) * lightColor * NdotL;

    // Specular
    float3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal, halfwayDir));
    float specPower = exp2(10.0 * (1.0 - roughness) + 1.0);
    float3 specular = pow(NdotH, specPower) * lightColor;

    return diffuse + specular;
}


