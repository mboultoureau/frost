cbuffer Constants
{
    float4x4 WorldViewProjection;
    float4x4 World;
    
    float4 LightPosition;
    float4 AmbientColor;
    float4 DiffuseColor;
    
    float4 EmissiveColor;
    
    float4 CameraPosition;
    
    int numberDiffuseTextures;
    int hasNormalMap;
    int hasEmissiveTexture;
    int hasAmbientOclusionTexture;
    
    int hasMetallicTexture;
    float roughnessValue;
    int hasRoughnessTexture;
    int padding[1];
}

struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};


struct PixelInput
{
    float4 position : SV_Position;
    float3 lightDirection : TEXCOORD0;
    float3 cameraDirection : TEXCOORD1;
    float2 texCoord : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
    float3 worldNormal : TEXCOORD5;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(input.position, WorldViewProjection);
    output.texCoord = input.texCoord;

    float3 worldPosition = mul(input.position, World).xyz;
    output.lightDirection = LightPosition.xyz - worldPosition;
    output.cameraDirection = CameraPosition.xyz - worldPosition;
    
    float3 N = normalize(mul(float4(input.normal, 0.0f), World).xyz);
    float3 T = normalize(mul(float4(input.tangent, 0.0f), World).xyz);
    float3 B = normalize(mul(float4(input.bitangent, 0.0f), World).xyz);
    
    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);
    
    output.worldNormal = N;
    output.worldTangent = T;
    output.worldBitangent = B;

    return output;
}