#include "BoostCommon.hlsli"

VS_Output main(VS_Input vin)
{
    VS_Output vout;

    float4 worldPos = mul(float4(vin.Position, 1.0f), WorldMatrix);

    vout.WorldPos = worldPos.xyz;
    vout.Normal = normalize(mul((float3x3) WorldMatrix, vin.Normal));
    vout.TexCoord = vin.TexCoord;

    float4 viewPos = mul(worldPos, ViewMatrix);
    vout.Position = mul(viewPos, ProjectionMatrix);

    vout.LocalPos = vin.Position;
    
    return vout;
}
