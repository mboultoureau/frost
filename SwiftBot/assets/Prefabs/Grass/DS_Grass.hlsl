#include "GrassCommon.hlsli"

[domain("tri")]
DSOutput main(HSConstantData input, float3 bary : SV_DomainLocation,
            const OutputPatch<HSOutput, 3> patch)
{
    DSOutput output;
    
    output.position = bary.x * patch[0].position +
                      bary.y * patch[1].position +
                      bary.z * patch[2].position;
    
    output.worldPos = bary.x * patch[0].worldPos +
                      bary.y * patch[1].worldPos +
                      bary.z * patch[2].worldPos;

    output.normal = bary.x * patch[0].normal +
                    bary.y * patch[1].normal +
                    bary.z * patch[2].normal;
    output.normal = normalize(output.normal);
    
    output.tangent = bary.x * patch[0].tangent +
                     bary.y * patch[1].tangent +
                     bary.z * patch[2].tangent;
    output.tangent = normalize(output.tangent);

    output.uv = bary.x * patch[0].uv +
                bary.y * patch[1].uv +
                bary.z * patch[2].uv;
    
    return output;
}