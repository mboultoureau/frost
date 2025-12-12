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
    // Read GBuffer
    float3 albedo = AlbedoTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 normal = normalize(NormalTexture.Sample(GBufferSampler, input.TexCoord).rgb * 2.0f - 1.0f);
    float3 worldPos = WorldPosTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float4 mat = MaterialTexture.Sample(GBufferSampler, input.TexCoord);
    float metal = mat.r;
    float rough = mat.g;
    float ao = mat.b;

    float3 viewDir = normalize(CameraPosition - worldPos);
    float3 currentLight = LuminanceTexture.Sample(GBufferSampler, input.TexCoord).rgb;

    // Point light (no shadow sampled here — point shadows need cubemap)
    float3 toL = Position - worldPos;
    float dist = length(toL);
    
    float3 dir = normalize(LightDirection);
    //bool isInDir = (dot(l, u) >= sqrt(2) / 2);
    
    float3 d = normalize(worldPos - Position);

    bool isInDir =
       (dir.x > 0 && d.x > abs(d.y) && d.x > abs(d.z)) ||
       (dir.x < 0 && -d.x > abs(d.y) && -d.x > abs(d.z)) ||
       (dir.y > 0 && d.y > abs(d.x) && d.y > abs(d.z)) ||
       (dir.y < 0 && -d.y > abs(d.x) && -d.y > abs(d.z)) ||
       (dir.z > 0 && d.z > abs(d.x) && d.z > abs(d.y)) ||
       (dir.z < 0 && -d.z > abs(d.x) && -d.z > abs(d.y));

    if (dist < Radius && isInDir)
    {
        float3 lightDir = normalize(toL);
        float atten = 1.0f - (dist / Radius);
        atten *= atten;
        float3 lightCol = Color * Intensity * atten;
        float2 shadowUV;
        float depth;
        bool isInCascad = IsInCascadeView(worldPos, shadowUV, depth);
        float shadowFactor = SampleCascadedShadowPCF(worldPos, normal, Position, LightDirection, ShadowResolution, shadowUV, depth);
        currentLight += CalculateBlinnPhong(lightDir, viewDir, normal, lightCol, metal, rough) * shadowFactor;
    }

    return float4(currentLight, 1.0f);
}
