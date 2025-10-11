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

struct PixelInput
{
    float4 position : SV_Position;
    float3 normalWorld : NORMAL;
    float3 positionWorld : TEXCOORD0;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    float3 N = normalize(input.normalWorld);
    float4 ambientTerm = AmbientColor;

    float diffuseFactor = max(0.0f, dot(N, -LightDirection));
    float4 diffuseTerm = LightColor * diffuseFactor;
    float4 specularTerm = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (diffuseFactor > 0.0f)
    {
        float3 V = normalize(CameraPosition - input.positionWorld);
        float3 R = normalize(reflect(LightDirection, N));
        float specularFactor = pow(max(0.0f, dot(R, V)), SpecularPower);
        specularTerm = LightColor * specularFactor;
    }

    float4 finalColor = ambientTerm + diffuseTerm + specularTerm;
    return finalColor;
}