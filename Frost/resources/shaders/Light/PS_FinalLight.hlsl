Texture2D AlbedoTexture : register(t0); // RGB: Albedo
Texture2D LightTexture : register(t1);

SamplerState GBufferSampler : register(s0);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};


float4 main(PS_Input input) : SV_TARGET
{
    float3 albedo = AlbedoTexture.Sample(GBufferSampler, input.TexCoord).rgb;
    float3 light = LightTexture.Sample(GBufferSampler, input.TexCoord).rgb;

    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    finalColor += albedo * light;
    
    return float4(finalColor, 1.0f);
}