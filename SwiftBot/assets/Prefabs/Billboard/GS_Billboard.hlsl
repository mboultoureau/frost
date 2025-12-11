#include "BillboardCommon.hlsli" 

void EmitVertex(float3 worldPos, float2 uv, float3 normal, inout TriangleStream<GSOutput> stream)
{
    GSOutput output;
    output.position = mul(float4(worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
    output.worldPos = worldPos;
    output.normal = normal;
    output.uv = uv;
    stream.Append(output);
}

[maxvertexcount(4)]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream, uint primID : SV_PrimitiveID)
{
    if (primID > 0)
        return;
    
    float3 center = input[0].worldPos;
    float3 normal = normalize(CameraPosition - center);
    
    float3 globalUp = float3(0, 1, 0);
    if (abs(dot(normal, globalUp)) > 0.99)
        globalUp = float3(0, 0, 1);

    float3 right = normalize(cross(globalUp, normal));
    float3 up = normalize(cross(normal, right));
    float halfSize = BillboardSize * 0.5f;
    
    float3 tl = center + (-right * halfSize) + (up * halfSize);
    float3 tr = center + (right * halfSize) + (up * halfSize);
    float3 bl = center + (-right * halfSize) + (-up * halfSize);
    float3 br = center + (right * halfSize) + (-up * halfSize);

    EmitVertex(tl, float2(0, 0), normal, triStream);
    EmitVertex(bl, float2(0, 1), normal, triStream);
    EmitVertex(tr, float2(1, 0), normal, triStream);
    EmitVertex(br, float2(1, 1), normal, triStream);

    triStream.RestartStrip();
}