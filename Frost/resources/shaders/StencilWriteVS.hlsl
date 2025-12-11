struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

cbuffer PerFrameBuffer : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

cbuffer PerObjectBuffer : register(b1)
{
    matrix World;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(float4(input.position, 1.0f), World);
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.position = mul(viewPos, ProjectionMatrix);
    return output;
}