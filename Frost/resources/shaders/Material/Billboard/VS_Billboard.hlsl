#include "BillboardCommon.hlsli"

VSOutput main(VSInput input)
{
    VSOutput output;
    
    float4 worldPos = mul(float4(input.position, 1.0), WorldMatrix);
    
    output.worldPos = worldPos.xyz;
    
    return output;
}