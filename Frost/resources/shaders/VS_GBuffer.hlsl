// G-Buffer Pass: Vertex Shader
cbuffer VS_PerFrameConstants : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

cbuffer VS_PerObjectConstants : register(b1)
{
    matrix World;
};

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 Tangent : TEXCOORD2;
    float3 Bitangent : TEXCOORD3;
    float2 TexCoord : TEXCOORD4;
};

PS_Input main(VS_Input input)
{
    PS_Input output;

    float4x4 worldViewProjection = mul(mul(World, ViewMatrix), ProjectionMatrix);
    
    output.Position = mul(float4(input.Position, 1.0f), worldViewProjection);
    output.WorldPos = mul(float4(input.Position, 1.0f), World).xyz;
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);
    output.Tangent = normalize(mul(float4(input.Tangent.xyz, 0.0f), World).xyz);
    output.Bitangent = normalize(cross(output.Normal, output.Tangent) * input.Tangent.w);
    output.TexCoord = input.TexCoord;
    
    return output;
}