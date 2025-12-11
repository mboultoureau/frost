cbuffer VS_ShadowConstants : register(b0)
{
    float4x4 World;
    float4x4 LightViewProj;
};

struct VSInput
{
    float3 Position : POSITION;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    output.Position = mul(worldPos, LightViewProj);

    return output;
}
