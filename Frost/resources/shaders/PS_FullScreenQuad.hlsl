struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

Texture2D sourceTexture : register(t0);
SamplerState sourceSampler : register(s0);

float4 main(VSOutput input) : SV_TARGET
{
    return sourceTexture.Sample(sourceSampler, input.texCoord);
}