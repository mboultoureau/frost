cbuffer Constants
{   
    float4x4 WorldViewProjection;
    float4x4 World;
    
    float3 LightDirection;
    float4 LightColor;
    float4 AmbientColor;
    float SpecularPower;
    
    float3 CameraPosition;
}

struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
};

struct PixelInput
{
    float4 position : SV_Position;
    float3 normalWorld : NORMAL;
    float3 positionWorld : TEXCOORD0;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(input.position, WorldViewProjection);
    output.positionWorld = mul(input.position, World).xyz;
    output.normalWorld = normalize(mul(input.normal, (float3x3) World));
    return output;
}