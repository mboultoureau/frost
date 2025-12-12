cbuffer PerFrame : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    float3 CameraPosition;
    float padding;
}

cbuffer PerObject : register(b1)
{
    matrix WorldMatrix;
}

cbuffer MaterialParams : register(b3)
{
    float Time;
    float3 Padding1;
    float TopAmplitude;
    float TopFrequency;
    float TopWaveLength;

    float SideAmplitude;
    float SideFrequency;

    float TessellationFactor;
    float BevelSize;
    float3 Padding; // padding to keep 16-byte alignment
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
    float3 LocalPos : POSITION1;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct DS_Output
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

struct HS_ConstantData
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};