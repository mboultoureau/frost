cbuffer TransformBuffer : register(b0)
{
    float4 viewport; 
    float4 color; 
    float rotation; 
    float3 padding; 
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
    
    
    float2 center_offset = float2(0.5f, 0.5f);
    float2 centered_pos = input.position.xy - center_offset;

    float cosA = cos(rotation);
    float sinA = sin(rotation);
    
    float2 rotated_pos;
    rotated_pos.x = centered_pos.x * cosA - centered_pos.y * sinA;
    rotated_pos.y = centered_pos.x * sinA + centered_pos.y * cosA;

    float2 final_pos = rotated_pos + center_offset;
    

    float2 screen_pos = (final_pos.xy * viewport.zw) + viewport.xy;
    
    output.position.x = screen_pos.x * 2.0f - 1.0f;
    output.position.y = 1.0f - (screen_pos.y * 2.0f); 
    output.position.z = 0.0f;
    output.position.w = 1.0f;
    
    output.texcoord = input.texcoord;

    return output;
}