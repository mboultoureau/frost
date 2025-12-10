#include "BillboardCommon.hlsli"

Texture2D BillboardTexture : register(t0);
SamplerState BillboardSampler : register(s0);

struct PS_Output
{
    float4 Color : SV_Target0;
};

PS_Output main(GSOutput input)
{
    PS_Output output;
    
    float4 textureColor = BillboardTexture.Sample(BillboardSampler, input.uv);
    
    if (textureColor.a < 0.05f)
    {
        discard;
    }
    
    output.Color = textureColor;
    
    return output;
}