// GS_Billboard.hlsl
#include "BillboardCommon.hlsli" 

void EmitVertex(float3 worldPos, float2 uv, inout TriangleStream<GSOutput> stream, matrix viewMatrix, matrix projectionMatrix, float3 centerWorldPos)
{
    GSOutput output;
    
    output.position = mul(float4(worldPos, 1.0), mul(viewMatrix, projectionMatrix));
    
    output.uv = uv;
    output.worldPos = centerWorldPos; 
    
    stream.Append(output);
}

[maxvertexcount(6)]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream, uint primID : SV_PrimitiveID)
{
   
    if (primID > 0)
    {
        return;
    }
    
    float3 worldCenter = input[0].worldPos;
    
    float3 viewVector = normalize(CameraPosition.xyz - worldCenter);
    
    float3 worldUp = float3(0.0, 1.0, 0.0);
    float3 worldRight;

    if (abs(dot(viewVector, worldUp)) > 0.999f)
    {
        worldUp = float3(1.0, 0.0, 0.0);
    }
    
    worldRight = normalize(cross(worldUp, viewVector)); // Droit
    worldUp = normalize(cross(viewVector, worldRight)); // Haut
    
    float halfSize = BillboardSize / 2.0f;
    
    float3 cornerTL = worldCenter + worldUp * halfSize - worldRight * halfSize; // Top Left (UV: 0, 0)
    float3 cornerTR = worldCenter + worldUp * halfSize + worldRight * halfSize; // Top Right (UV: 1, 0)
    float3 cornerBL = worldCenter - worldUp * halfSize - worldRight * halfSize; // Bottom Left (UV: 0, 1)
    float3 cornerBR = worldCenter - worldUp * halfSize + worldRight * halfSize; // Bottom Right (UV: 1, 1)

    matrix V = ViewMatrix;
    matrix P = ProjectionMatrix;
    
    // Triangle 1 : BL, TL, BR
    EmitVertex(cornerBL, float2(0.0, 1.0), triStream, V, P, worldCenter);
    EmitVertex(cornerTL, float2(0.0, 0.0), triStream, V, P, worldCenter);
    EmitVertex(cornerBR, float2(1.0, 1.0), triStream, V, P, worldCenter);
    triStream.RestartStrip();

    // Triangle 2 : TR, BR, TL
    EmitVertex(cornerTR, float2(1.0, 0.0), triStream, V, P, worldCenter);
    EmitVertex(cornerBR, float2(1.0, 1.0), triStream, V, P, worldCenter);
    EmitVertex(cornerTL, float2(0.0, 0.0), triStream, V, P, worldCenter);
}