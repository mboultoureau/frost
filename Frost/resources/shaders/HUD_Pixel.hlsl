SamplerState samplerState : register(s0);

Texture2D shaderTexture : register(t0);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};


float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return shaderTexture.Sample(samplerState, input.texcoord);
}