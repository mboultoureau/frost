Texture2D BillboardTexture : register(t0);
SamplerState BillboardSampler : register(s0);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 WorldPosition : POSITION0;
};

float4 main(PS_Input input) : SV_TARGET
{
    float4 sampledColor = BillboardTexture.Sample(BillboardSampler, input.TexCoord);

    if (sampledColor.a < 0.01f)
        discard;

    return sampledColor;
}