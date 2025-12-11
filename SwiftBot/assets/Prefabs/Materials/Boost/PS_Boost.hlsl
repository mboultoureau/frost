#include "BoostCommon.hlsli"

struct PS_Output
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Material : SV_Target3;
};

PS_Output main(VS_Output input)
{
    PS_Output output;

    float3 F = normalize(Direction);
    float3 U = float3(0, 1, 0);

    if (abs(dot(F, U)) > 0.99f)
        U = float3(1, 0, 0);

    float3 R = normalize(cross(U, F));
    U = normalize(cross(F, R));

    float x = dot(input.WorldPos, R);
    float y = dot(input.WorldPos, F);
    float z = dot(input.WorldPos, U);

    float waveX = abs(frac(x * Frequency) * 2 - 1);
    float waveZ = abs(frac(z * Frequency) * 2 - 1);
    float wave = waveX * waveZ;

    float t = abs(frac(y * Frequency + Time) * 2 - 1);

    float3 Color1 = float3(1, 1, 0); // jaune
    float3 Color2 = float3(1, 0, 0); // rouge

    float3 boostColor = lerp(Color1, Color2, t) * (0.5 + 0.5 * wave);

    output.Albedo = float4(boostColor, 1.0f);
    output.Normal = float4(normalize(input.Normal) * 0.5f + 0.5f, 1.0f);
    output.WorldPos = float4(input.WorldPos, 1.0f);
    output.Material = float4(0.0f, 0.1f, 0.0f, 0.0f);

    return output;
}
