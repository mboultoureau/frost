cbuffer FontColorBuffer : register(b0)
{
    float4 viewport;
    float4 fontColor;
};

SamplerState samplerState : register(s0);
Texture2D shaderTexture : register(t0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float alpha = shaderTexture.Sample(samplerState, input.texcoord).r;
    
    return fontColor * alpha;
}