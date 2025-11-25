#include "WavesCommon.hlsli"

[domain("tri")]
DS_Output main(HS_ConstantData input, float3 domain : SV_DomainLocation, const OutputPatch<HS_Input, 3> patch)
{
    DS_Output Output;

    float3 worldPos = patch[0].WorldPos * domain.x + patch[1].WorldPos * domain.y + patch[2].WorldPos * domain.z;
    float2 texCoord = patch[0].TexCoord * domain.x + patch[1].TexCoord * domain.y + patch[2].TexCoord * domain.z;
    float3 normal = patch[0].Normal * domain.x + patch[1].Normal * domain.y + patch[2].Normal * domain.z;

    float waveX = worldPos.x * Frequency + Time;
    float waveZ = worldPos.z * Frequency + Time;
    
    float displacement = Amplitude * (sin(waveX) + cos(waveZ));
    worldPos.y += displacement;

    float dYdX = Amplitude * Frequency * cos(waveX);
    float dYdZ = Amplitude * Frequency * -sin(waveZ);
    
    float3 tangentX = float3(1.0, dYdX, 0.0);
    float3 tangentZ = float3(0.0, dYdZ, 1.0);
    
    Output.Normal = normalize(cross(tangentZ, tangentX));
    Output.Position = mul(float4(worldPos, 1.0f), mul(ViewMatrix, ProjectionMatrix));
    Output.WorldPos = worldPos;
    Output.TexCoord = texCoord;

    return Output;
}