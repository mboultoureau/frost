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

    // Distance to each player camera
    float3 row0 = ViewMatrix[0].xyz;
    float3 row1 = ViewMatrix[1].xyz;
    float3 row2 = ViewMatrix[2].xyz;
    float3 trans = float3(ViewMatrix[0].w, ViewMatrix[1].w, ViewMatrix[2].w);

    float3 camPos;
    camPos.x = -dot(row0, trans);
    camPos.y = -dot(row1, trans);
    camPos.z = -dot(row2, trans);

    outCP.DistToPlayer = distance(outCP.WorldPos, camPos);

    return outCP;
}