#include "WaterCommon.hlsli"

HS_Input main(VS_Input vin)
{
    HS_Input outCP;
    outCP.LocalPos = vin.Position;
    
    float4 worldPos4 = mul(float4(vin.Position, 1.0f), WorldMatrix);
    outCP.WorldPos = worldPos4.xyz;
    
    outCP.Normal = normalize(mul((float3x3) WorldMatrix, vin.Normal));
    outCP.TexCoord = vin.TexCoord;

    return outCP;
}