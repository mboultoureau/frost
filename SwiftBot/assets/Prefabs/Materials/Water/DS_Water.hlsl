#include "WaterCommon.hlsli"

[domain("tri")]
DS_Output main(HS_ConstantData patchConst, float3 bary : SV_DomainLocation, const OutputPatch<HS_Input, 3> patch)
{
    DS_Output outV;

    // Barycentric interpolation of control point data
    float3 worldPos = patch[0].WorldPos * bary.x + patch[1].WorldPos * bary.y + patch[2].WorldPos * bary.z;
    float3 localPos = patch[0].LocalPos * bary.x + patch[1].LocalPos * bary.y + patch[2].LocalPos * bary.z;
    float2 texCoord = patch[0].TexCoord * bary.x + patch[1].TexCoord * bary.y + patch[2].TexCoord * bary.z;
    float3 normal = normalize(patch[0].Normal * bary.x + patch[1].Normal * bary.y + patch[2].Normal * bary.z);

    // --- Wave calculation 
    float topWave = sin(TopWaveLength * worldPos.z + TopFrequency * Time) * TopAmplitude;

    float sideX = sin((localPos.x + localPos.z) + SideFrequency * Time) * SideAmplitude;
    float sideY = cos(localPos.y + SideFrequency * Time) * SideAmplitude;
    float sideSum = sideX * sideY;

    float topBlend = saturate(normal.y)*2;

    float displacement = lerp(sideSum, topWave, topBlend);
    
    // --- Bevel calculation
    
    float bevelDist;

    float3 absN = abs(normal);

    if (absN.x > absN.y && absN.x > absN.z)
    {
        float2 p = localPos.yz;

        float2 d0 = (0.5 - p) / BevelSize;
        float2 d1 = (p + 0.5) / BevelSize;

        bevelDist = min(min(d0.x, d0.y), min(d1.x, d1.y));
    }
    else if (absN.y > absN.x && absN.y > absN.z)
    {
        float2 p = localPos.xz;

        float2 d0 = (0.5 - p) / BevelSize;
        float2 d1 = (p + 0.5) / BevelSize;

        bevelDist = min(min(d0.x, d0.y), min(d1.x, d1.y));
    }
    else
    {
        float2 p = localPos.xy;

        float2 d0 = (0.5 - p) / BevelSize;
        float2 d1 = (p + 0.5) / BevelSize;

        bevelDist = min(min(d0.x, d0.y), min(d1.x, d1.y));
    }

    bevelDist = saturate(bevelDist);

    worldPos.y += bevelDist * displacement;

    // --- Normal calculation (analytical approximation) ---
    
    float dh_dx = lerp(
        cos(worldPos.y + SideFrequency * Time)*sin((worldPos.x + worldPos.z) + SideFrequency * Time) * SideAmplitude,
        - sin(TopWaveLength * worldPos.z + TopFrequency * Time) * TopAmplitude,
        topBlend);
    
    float dh_dz = lerp(
        cos(worldPos.y + SideFrequency * Time) * cos((worldPos.x + worldPos.z) + SideFrequency * Time) * SideAmplitude,
        -cos(TopWaveLength * worldPos.z + TopFrequency * Time) * TopAmplitude,
        topBlend);

    float dh_dy = lerp(
        sin(worldPos.y + SideFrequency * Time) * sin((worldPos.x + worldPos.z) + SideFrequency * Time) * SideAmplitude,
        -sin(TopWaveLength * worldPos.z + TopFrequency * Time) * TopAmplitude,
        topBlend);


    //---------------------------------------------
    // Compute geometric normal
    //---------------------------------------------
    float3 displacedNormal = normalize(normal + float3(
        dh_dx,
        dh_dy,
        dh_dz
    ));

    outV.WorldPos = worldPos;
    outV.Normal = displacedNormal;
    outV.TexCoord = texCoord;
    outV.Position = mul(mul(float4(worldPos, 1.0f), ViewMatrix), ProjectionMatrix);

    return outV;
}