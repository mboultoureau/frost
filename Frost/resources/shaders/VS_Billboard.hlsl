cbuffer VS_PerFrameConstants : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

cbuffer BillboardConstants : register(b1)
{
    float3 Position; 
    float Width;
    
    float Height;
    float CenteredY;
    float2 Padding;
};

struct VS_Input
{
    uint VertexID : SV_VertexID;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 WorldPosition : POSITION0;
};

VS_Output main(VS_Input input)
{
    VS_Output output;
    
    float2 quadCoords[6] =
    {
        float2(-1.0f, -1.0f), float2(-1.0f, 1.0f), float2(1.0f, -1.0f),
        float2(1.0f, -1.0f), float2(-1.0f, 1.0f), float2(1.0f, 1.0f)
    };
    
    float2 texCoords[6] =
    {
        float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f),
        float2(1.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f)
    };
    
    float2 quad = quadCoords[input.VertexID];
    
    float3 quadOffset = float3(quad.x * Width / 2.0f,
                               quad.y * Height / 2.0f - CenteredY,
                               0.0f);
    
    float4 billboardViewCenter = mul(float4(Position, 1.0f), ViewMatrix);

    
    const float Z_BIAS = 1.0f; 
    
    float4 finalViewPosition = billboardViewCenter + float4(quadOffset.x, quadOffset.y, Z_BIAS, 0.0f);

    output.Position = mul(finalViewPosition, ProjectionMatrix);
    
    output.TexCoord = texCoords[input.VertexID];
    output.WorldPosition = Position.xyz;
    
    return output;
}