Texture2D SourceTexture     : register(t0);
SamplerState SourceSampler  : register(s0);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer ChromaticAberrationConstants : register(b0)
{
    float2  CenterRed;
    float2  CenterGreen;
    float2  CenterBlue;
    float   Strength;
};

float4 main(PS_Input input) : SV_TARGET
{
    float2 redDirection = input.TexCoord - CenterRed;
    float2 greenDirection = input.TexCoord - CenterGreen;
    float2 blueDirection = input.TexCoord - CenterBlue;
    
    float2 redTexCoord = input.TexCoord + redDirection * Strength;
    float2 greenTexCoord = input.TexCoord + greenDirection * Strength;
    float2 blueTexCoord = input.TexCoord + blueDirection * Strength;
    
    float red = SourceTexture.Sample(SourceSampler, redTexCoord).r;
    float green = SourceTexture.Sample(SourceSampler, greenTexCoord).g;
    float blue = SourceTexture.Sample(SourceSampler, blueTexCoord).b;

    return float4(red, green, blue, 1.0);
}