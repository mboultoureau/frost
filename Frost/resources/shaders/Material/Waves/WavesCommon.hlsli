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
    float Time;
    float Amplitude;
    float Frequency;
    float TessellationFactor;
    float3 Padding;
}

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Tangent : TANGENT;
};

struct HS_Input
{
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct DS_Output
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct HS_ConstantData
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};