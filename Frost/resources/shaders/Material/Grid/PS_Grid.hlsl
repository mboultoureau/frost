cbuffer GridParameters : register(b3)
{
    float4 GridColor;
    float4 BaseColor;
    float GridScale;
    float LineThickness;
    float2 Padding;
};

struct PS_Input
{
    float4 PositionCS : SV_POSITION;
    float3 PositionWS : POSITION;
    float3 NormalWS : NORMAL;
};

struct PS_Output
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Material : SV_Target3;
};

float GridFactor(float3 worldPos, float scale, float thickness)
{
    float2 coord = worldPos.xz * scale;
    float2 derivative = fwidth(coord);
    float2 gridDist = abs(frac(coord - 0.5) - 0.5);
    float2 lines = 1.0 - smoothstep(thickness * 0.5 - derivative, thickness * 0.5 + derivative, gridDist);
    return max(lines.x, lines.y);
}

PS_Output main(PS_Input input)
{
    PS_Output output;

    float intensity = GridFactor(input.PositionWS, GridScale, LineThickness);
    float3 finalColor = lerp(BaseColor.rgb, GridColor.rgb, intensity);

    output.Albedo = float4(finalColor, 1.0);
    
    output.Normal = float4(normalize(input.NormalWS) * 0.5f + 0.5f, 1.0);
    output.WorldPos = float4(input.PositionWS, 1.0);
    output.Material = float4(0.9f, 0.0f, 1.0f, 0.0f);

    return output;
}