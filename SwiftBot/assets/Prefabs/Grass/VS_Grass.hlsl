#include "GrassCommon.hlsli"

VSOutput main(VSInput input)
{
    VSOutput output;
    
    float3 world = mul(float4(input.position, 1.0), WorldMatrix).xyz;
    
    output.position = input.position;
    output.worldPos = world;
    output.normal = normalize(mul((float3x3) WorldMatrix, input.normal));
    output.tangent = input.tangent;
    output.uv = input.uv;
    
    return output;
}