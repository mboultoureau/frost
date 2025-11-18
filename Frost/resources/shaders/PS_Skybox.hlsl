TextureCube SkyboxTexture   : register(t0);
SamplerState SkyboxSampler  : register(s0);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD0;
};

float4 main(PS_Input input) : SV_TARGET
{
    return SkyboxTexture.Sample(SkyboxSampler, input.TexCoord);
}