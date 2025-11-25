#include "WavesCommon.hlsli"

HS_ConstantData CalcHSPatchConstants(InputPatch<HS_Input, 3> patch, uint PatchID : SV_PrimitiveID)
{
    HS_ConstantData Output;
    
    Output.EdgeTess[0] = TessellationFactor;
    Output.EdgeTess[1] = TessellationFactor;
    Output.EdgeTess[2] = TessellationFactor;
    Output.InsideTess = TessellationFactor;
    
    return Output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_Input main(InputPatch<HS_Input, 3> patch, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
    HS_Input Output;
    Output.WorldPos = patch[i].WorldPos;
    Output.Normal = patch[i].Normal;
    Output.TexCoord = patch[i].TexCoord;
    return Output;
}