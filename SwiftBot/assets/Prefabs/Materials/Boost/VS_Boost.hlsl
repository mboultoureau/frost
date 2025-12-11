#include "BoostCommon.hlsli"

VS_Output main(VS_Input vin)
{
    VS_Output vout;

    float4 worldPos = mul(float4(vin.Position, 1.0f), WorldMatrix);

    vout.WorldPos = worldPos.xyz;
    vout.Normal = normalize(mul((float3x3) WorldMatrix, vin.Normal));
    vout.TexCoord = vin.TexCoord;

    vout.Position = mul(worldPos, ViewMatrix);
    vout.Position = mul(vout.Position, ProjectionMatrix);

    return vout;
}
