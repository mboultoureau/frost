cbuffer PerFrame : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

cbuffer PerObject : register(b1)
{
    matrix WorldMatrix;
}

cbuffer MaterialParams : register(b3)
{
    float4 CameraPosition;
 
    float BillboardSize;
    float3 padding;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float3 worldPos : WORLDPOS;
};

struct GSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0; 
};