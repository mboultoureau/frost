cbuffer TransformBuffer : register(b0)
{
    float4 viewport;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float2 screen_pos = (input.position.xy * viewport.zw) + viewport.xy;
    
    output.position.x = screen_pos.x * 2.0f - 1.0f;
    output.position.y = 1.0f - (screen_pos.y * 2.0f);
    output.position.z = 0.0f;
    output.position.w = 1.0f;
    
    output.texcoord = input.texcoord;

    return output;
}