

#include "BillboardCommon.hlsli" 

void EmitVertex(float3 worldPos, float2 uv, inout TriangleStream<GSOutput> stream, matrix viewMatrix, matrix projectionMatrix)
{
    GSOutput output;
    output.position = mul(float4(worldPos, 1.0), mul(viewMatrix, projectionMatrix));
    output.uv = uv;
    output.worldPos = worldPos;
    stream.Append(output);
}

[maxvertexcount(4)] 
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream, uint primID : SV_PrimitiveID)
{
    if (primID > 0)
    {
        return;
    }
    float3 worldCenter = input[0].worldPos;
    
    
    float3 worldUp = float3(0.0, 1.0, 0.0);

   
    float3 viewDirection = normalize(CameraPosition.xyz - worldCenter);
    
    
    float3 worldRight = cross(worldUp, viewDirection);
    worldRight = normalize(worldRight);
    
   
    float halfSize = BillboardSize / 2.0f;
    
    
    float3 cornerTL = worldCenter - worldRight * halfSize + worldUp * halfSize; // Top Left (UV: 0, 0)
    float3 cornerTR = worldCenter + worldRight * halfSize + worldUp * halfSize; // Top Right (UV: 1, 0)
    float3 cornerBL = worldCenter - worldRight * halfSize - worldUp * halfSize; // Bottom Left (UV: 0, 1)
    float3 cornerBR = worldCenter + worldRight * halfSize - worldUp * halfSize; // Bottom Right (UV: 1, 1)

    
    EmitVertex(cornerTL, float2(0, 0), triStream, ViewMatrix, ProjectionMatrix); // TL
    EmitVertex(cornerBL, float2(0, 1), triStream, ViewMatrix, ProjectionMatrix); // BL
    EmitVertex(cornerTR, float2(1, 0), triStream, ViewMatrix, ProjectionMatrix); // TR
    EmitVertex(cornerBR, float2(1, 1), triStream, ViewMatrix, ProjectionMatrix); // BR

    triStream.RestartStrip();
}