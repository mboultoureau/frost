#include "GrassCommon.hlsli"

HSConstantData HS_PatchConstants(InputPatch<HSOutput, 3> patch, uint PatchID : SV_PrimitiveID)
{
    HSConstantData pc;

    float3 patchCenter = (patch[0].worldPos + patch[1].worldPos + patch[2].worldPos) / 3.0f;

    float dist = distance(CameraPosition, patchCenter);

    float NearDist = 50.0f;
    float FarDist = 100.0f;
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
HSOutput main(InputPatch<HSOutput, 3> patch, uint cpID : SV_OutputControlPointID)
{
    return patch[cpID];
}