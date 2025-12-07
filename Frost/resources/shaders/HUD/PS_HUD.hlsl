cbuffer ConstantBuffer : register(b0)
{
    float4 Viewport;
    float4 Color;
    float Rotation;
};

Texture2D shaderTexture : register(t0);
SamplerState samplerState : register(s0);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    float4 textureColor = shaderTexture.Sample(samplerState, input.texcoord);
    float4 finalColor = textureColor * Color;
    
    return finalColor;
}