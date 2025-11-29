Texture2D SourceTexture     : register(t0);
SamplerState SourceSampler  : register(s0);

Texture2D DepthTexture      : register(t1);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer UnderWaterConstants : register(b0)
{
    float MinDepth;
    float Strength;
    float red;
    float green;
    float blue;
};

float4 main(PS_Input input) : SV_TARGET
{    
    float4 currentColor = SourceTexture.Sample(SourceSampler, input.TexCoord);
    float depth = saturate(Strength * (DepthTexture.Sample(SourceSampler, input.TexCoord).r - MinDepth) / (1 - MinDepth));
    float3 newColor = lerp(currentColor.rgb, float3(red, green, blue), depth);
    return saturate(float4(newColor.xyz, 1.0));
}