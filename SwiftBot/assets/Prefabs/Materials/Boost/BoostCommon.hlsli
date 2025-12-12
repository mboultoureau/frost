// BoostCommon.hlsli

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
    float Amplitude;
    float Frequency;
    float padding1;
}

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float3 LocalPos : TEXCOORD3;
};
