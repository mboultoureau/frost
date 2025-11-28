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
    float4 TopColor;
    float4 BottomColor;
    
    float Time;
    
    float BladeWidth;
    float BladeWidthRandom;
    float BladeHeight;
    float BladeHeightRandom;
    
    float BladeForward;
    float BladeCurve;
    float BendRotationRandom;
    
    float TessellationFactor;
    
    float WindStrength;
    
    float3 CameraPosition;
    
    float3 padding;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct VSOutput
{
    float3 position : POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct GSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

struct HSOutput
{
    float3 position : POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct HSConstantData
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

struct DSOutput
{
    float3 position : POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

// Remplace rand() d'Unity
float rand(float3 seed)
{
    return frac(sin(dot(seed, float3(12.9898, 78.233, 45.164))) * 43758.5453);
}

// Remplace AngleAxis3x3 d'Unity
float3x3 AngleAxis3x3(float angle, float3 axis)
{
    float c, s;
    sincos(angle, s, c);
    
    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    
    return float3x3(
        t * x * x + c, t * x * y - s * z, t * x * z + s * y,
        t * x * y + s * z, t * y * y + c, t * y * z - s * x,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c
    );
}