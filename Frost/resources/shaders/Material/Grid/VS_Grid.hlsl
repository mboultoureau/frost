cbuffer VS_PerFrameConstants : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

cbuffer VS_PerObjectConstants : register(b1)
{
    matrix WorldMatrix;
};

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Tangent : TANGENT;
};

struct VS_Output
{
    float4 PositionCS : SV_POSITION;
    float3 PositionWS : POSITION;
    float3 NormalWS : NORMAL;
};

VS_Output main(VS_Input input)
{
    VS_Output output;
    
    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.PositionWS = worldPos.xyz;
    
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.PositionCS = mul(viewPos, ProjectionMatrix);
    
    output.NormalWS = normalize(mul(input.Normal, (float3x3) WorldMatrix));
    
    return output;
}