#include "BillboardCommon.hlsli"

VSOutput main(VSInput input)
{
    VSOutput output;
    
    float3 objectCenter = float3(WorldMatrix[3][0], WorldMatrix[3][1], WorldMatrix[3][2]);
    output.worldPos = objectCenter;
    
    return output;
}