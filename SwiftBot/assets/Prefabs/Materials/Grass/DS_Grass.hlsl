#include "GrassCommon.hlsli"

[domain("tri")]
DSOutput main(HSConstantData input, float3 barycentricCoords : SV_DomainLocation,
            const OutputPatch<HSOutput, 3> patch)
{
    DSOutput output;
    
    // Interpolation barycentrique des attributs
    output.position = barycentricCoords.x * patch[0].position +
                      barycentricCoords.y * patch[1].position +
                      barycentricCoords.z * patch[2].position;
    
    output.normal = barycentricCoords.x * patch[0].normal +
                    barycentricCoords.y * patch[1].normal +
                    barycentricCoords.z * patch[2].normal;
    output.normal = normalize(output.normal);
    
    output.tangent = barycentricCoords.x * patch[0].tangent +
                     barycentricCoords.y * patch[1].tangent +
                     barycentricCoords.z * patch[2].tangent;
    output.tangent = normalize(output.tangent);
    
    output.worldPos = barycentricCoords.x * patch[0].worldPos +
                      barycentricCoords.y * patch[1].worldPos +
                      barycentricCoords.z * patch[2].worldPos;
    
    return output;
}