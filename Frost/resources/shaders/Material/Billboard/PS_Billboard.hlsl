// PS_Billboard.hlsl
#include "BillboardCommon.hlsli"

Texture2D BillboardTexture : register(t0);
SamplerState BillboardSampler : register(s0);

struct PS_Output
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Material : SV_Target3;
};

PS_Output main(GSOutput input)
{
    PS_Output output;
    
    float4 textureColor = BillboardTexture.Sample(BillboardSampler, input.uv);
    
    if (textureColor.a < 0.05f)
    {
        discard;
    }
    
    output.Albedo = textureColor;
    
    float3 normalWorld = normalize(CameraPosition.xyz - input.worldPos);
    
    output.Normal = float4(normalWorld * 0.5f + 0.5f, 1.0f);
    
    output.WorldPos = float4(input.worldPos, 1.0f);
    
    output.Material = float4(0.0f, 0.0f, 0.0f, 1.0f);

    return output;
}