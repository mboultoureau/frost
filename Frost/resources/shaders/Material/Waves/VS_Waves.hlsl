#include "WavesCommon.hlsli"

HS_Input main(VS_Input input)
{
    HS_Input output;

    output.WorldPos = mul(float4(input.Position, 1.0f), WorldMatrix).xyz;
    output.Normal = mul(input.Normal, (float3x3) WorldMatrix);
    output.TexCoord = input.TexCoord;

    return output;
}