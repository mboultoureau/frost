#include "GrassCommon.hlsli"

float CalcTessFactor(float3 pA, float3 pB)
{
    float edgeLen = distance(pA, pB);
    
    float3 center = (pA + pB) * 0.5f;
    float distToCam = distance(CameraPosition, center);
    
    float lodFactor = 1.0f - saturate((distToCam - LodNear) / (LodFar - LodNear));
    lodFactor = pow(lodFactor, 2.0f);
    
    float tess = edgeLen * GrassDensity * ParentScale * lodFactor;
    
    return clamp(tess, 2.0f, 64.0f);
}

HSConstantData HS_PatchConstants(InputPatch<HSOutput, 3> patch, uint PatchID : SV_PrimitiveID)
{
    HSConstantData pc;
    pc.EdgeTess[0] = CalcTessFactor(patch[1].worldPos, patch[2].worldPos);
    pc.EdgeTess[1] = CalcTessFactor(patch[2].worldPos, patch[0].worldPos);
    pc.EdgeTess[2] = CalcTessFactor(patch[0].worldPos, patch[1].worldPos);
    pc.InsideTess = (pc.EdgeTess[0] + pc.EdgeTess[1] + pc.EdgeTess[2]) / 3.0f;
    return pc;
}

[outputcontrolpoints(3)]
[partitioning("fractional_odd")]
[domain("tri")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HS_PatchConstants")]
HSOutput main(InputPatch<HSOutput, 3> patch, uint cpID : SV_OutputControlPointID)
{
    return patch[cpID];
}