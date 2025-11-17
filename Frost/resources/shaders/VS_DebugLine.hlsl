cbuffer VS_PerFrameConstants : register(b0)
{
    float4x4 ViewProjectionMatrix;
};

cbuffer VS_PerObjectConstants : register(b1)
{
    float4x4 WorldMatrix;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.Position = mul(output.Position, ViewProjectionMatrix);
    output.Color = input.Color;
    return output;
}