#include "WaterCommon.hlsli"

HS_ConstantData HS_PatchConstants(InputPatch<HS_Input, 3> patch, uint PatchID : SV_PrimitiveID)
{
    HS_ConstantData pc;
    
    float3 patchCenter = (patch[0].WorldPos + patch[1].WorldPos + patch[2].WorldPos) / 3.0f;
    
    float dist = distance(CameraPosition, patchCenter);

    float NearDist = 25.0f;
    float FarDist = 800.0f;
    float MinTess = 1.0f;
    float MaxTess = TessellationFactor;

    float t = saturate((FarDist - dist) / (FarDist - NearDist));
    float tess = lerp(MinTess, MaxTess, t);
    
    pc.EdgeTess[0] = tess;
    pc.EdgeTess[1] = tess;
    pc.EdgeTess[2] = tess;
    pc.InsideTess = tess;

    return pc;
}

[outputcontrolpoints(3)]
[partitioning("integer")]
[domain("tri")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HS_PatchConstants")]
HS_Input main(InputPatch<HS_Input, 3> patch, uint cpID : SV_OutputControlPointID)
{
    // Pass-through control point
    return patch[cpID];
}


