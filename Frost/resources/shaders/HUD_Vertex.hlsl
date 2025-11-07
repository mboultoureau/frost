cbuffer TransformBuffer : register(b0)
{
    matrix world;
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

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    
    
    float4 worldPos = mul(float4(input.position.x, input.position.y, input.position.z, 1.0f), world);
    
   
    float x_ndc = (2.0f * worldPos.x) - 1.0f;
    
    float y_ndc = 1.0f - (2.0f * worldPos.y);
    
    output.position = float4(x_ndc, y_ndc, 0.0f, 1.0f);

    output.texcoord = input.texcoord;

    return output;
}