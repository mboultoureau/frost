#include "GrassCommon.hlsli"

VSOutput main(VSInput input)
{
    VSOutput output;
    
    float3 world = mul(float4(input.position, 1.0), WorldMatrix).xyz;
    
    output.position = input.position;
    output.worldPos = world;
    output.normal = input.normal;
    output.tangent = input.tangent;
    
    return output;
}