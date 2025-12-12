#include "GrassCommon.hlsli"

struct PS_Output
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Material : SV_Target3;
};

PS_Output main(GSOutput input)
{
    PS_Output output;
    
    float4 grassColor = lerp(BottomColor, TopColor, input.uv.y);
    
    output.Albedo = grassColor;
    output.Normal = float4(normalize(input.normal) * 0.5f + 0.5f, 1.0f);
    output.WorldPos = float4(input.worldPos, 1.0f);
    output.Material = float4(0.0f, 0.8f, 1.0f, 0.0f);

    return output;
}