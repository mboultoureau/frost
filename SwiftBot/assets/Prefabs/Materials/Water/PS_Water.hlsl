#include "WaterCommon.hlsli"

struct PS_Output
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Material : SV_Target3;
};

PS_Output main(DS_Output input)
{
    PS_Output output;
    
    output.Albedo = float4(0.0f, 0.4f, 0.8f, .5f);
    output.Normal = float4(normalize(input.Normal) * 0.5f + 0.5f, 1.0f);
    output.WorldPos = float4(input.WorldPos, 1.0f);
    output.Material = float4(0.0f, 0.1f, 0.0f, 0.f);

    return output;
}