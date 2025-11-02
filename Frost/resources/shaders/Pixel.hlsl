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

Texture2D diffuseTexture : register(t0);
SamplerState textureSampler : register(s0);

struct PixelInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 lightDirection : TEXCOORD0;
    float3 cameraDirection : TEXCOORD1;
    float2 texCoord : TEXCOORD2;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    float4 BaseColor = DiffuseColor;
    if (numberDiffuseTextures > 0)
    {
        BaseColor *= diffuseTexture.Sample(textureSampler, input.texCoord);
    }
    
    float3 color;
    
    float3 N = normalize(input.normal);
    float3 L = normalize(input.lightDirection);
    float3 V = normalize(input.cameraDirection);
    
    float3 diff = saturate(dot(N, L));
    float3 R = normalize(2 * diff.xyz * N - L);
    float3 S = pow(saturate(dot(R, V)), 4);
    
    color = (AmbientColor.rgb * BaseColor.rgb) + (BaseColor.rgb * diff) + S;
    return float4(color, 1.0f);
}
