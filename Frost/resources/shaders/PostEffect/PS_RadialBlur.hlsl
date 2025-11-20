Texture2D SourceTexture     : register(t0);
SamplerState SourceSampler  : register(s0);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer RadialBlurConstants : register(b0)
{
    float2  Center;
    float   Strength;
    int     SampleCount;
};

float4 main(PS_Input input) : SV_TARGET
{
    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    float2 direction = input.TexCoord - Center;
    float step = length(direction) * Strength / SampleCount;
    
    for (int i = 0; i < SampleCount; i++)
    {
        float2 sampleCoord = input.TexCoord + normalize(direction) * step * i;
        finalColor += SourceTexture.Sample(SourceSampler, sampleCoord);
    }
    
    return finalColor / SampleCount;
}