cbuffer Constants
{
    float4x4 WorldViewProjection;
    float4x4 World;
    
    float4 LightPosition;
    float4 AmbientColor;
    float4 DiffuseColor;
    
    float4 CameraPosition;
    
    int numberDiffuseTextures;
    float3 padding;
}

struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};


struct PixelInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 lightDirection : TEXCOORD0;
    float3 cameraDirection : TEXCOORD1;
    float2 texCoord : TEXCOORD2;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(input.position, WorldViewProjection);
    output.normal = mul(float4(input.normal, 0.0f), World).xyz;
    
    float3 worldPosition = mul(input.position, World).xyz;
    output.lightDirection = LightPosition.xyz - worldPosition;
    output.cameraDirection = CameraPosition.xyz - worldPosition;
    output.texCoord = input.texCoord;
    
    return output;
}