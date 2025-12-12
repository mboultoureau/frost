#include "WaterCommon.hlsli"

HS_Input main(VS_Input vin)
{
    HS_Input outCP;
    // compute world-space position for control points
    outCP.LocalPos = vin.Position;
    float4 worldPos4 = mul(float4(vin.Position, 1.0f), WorldMatrix);
    outCP.WorldPos = worldPos4.xyz;
    // transform normal (no scaling accounted for; assume uniform scale or provide normal matrix)
    outCP.Normal = normalize(mul((float3x3) WorldMatrix, vin.Normal));
    outCP.TexCoord = vin.TexCoord;

    // distance to camera (PlayerPosition holds camera pos)
    outCP.DistToPlayer = distance(outCP.WorldPos, camPos);

    return outCP;
}