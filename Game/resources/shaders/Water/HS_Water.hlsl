#include "WaterCommon.hlsli"

HS_ConstantData HS_PatchConstants(InputPatch<HS_Input, 3> patch, uint PatchID : SV_PrimitiveID)
{
    HS_ConstantData pc;

    float3 patchCenter = (patch[0].WorldPos + patch[1].WorldPos + patch[2].WorldPos) / 3.0f;
    
    float3 row0 = ViewMatrix[0].xyz;
    float3 row1 = ViewMatrix[1].xyz;
    float3 row2 = ViewMatrix[2].xyz;

    float3 trans = float3(ViewMatrix[0].w, ViewMatrix[1].w, ViewMatrix[2].w);

    float3 camPos;
    camPos.x = -dot(row0, trans);
    camPos.y = -dot(row1, trans);
    camPos.z = -dot(row2, trans);

    float dist = distance(camPos, patchCenter);

    float NearDist = 10.0f;
    float FarDist = 100.0f;
    float MinTess = 1.0f;
    float MaxTess = TessellationFactor;

    float t = saturate((FarDist - dist) / (FarDist - NearDist));

    //t = pow(t, 2.0f);

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


