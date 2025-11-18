cbuffer VS_SkyboxConstants : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

struct VS_Input
{
    float3 Position : POSITION;
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD0;
};

PS_Input main(VS_Input input)
{
    PS_Input output;

    output.TexCoord = input.Position;

    // Remove translation to be always aligned with the camera
    matrix view = ViewMatrix;
    view[3][0] = 0.0f;
    view[3][1] = 0.0f;
    view[3][2] = 0.0f;
    
    // Depth value is maximum
    float4 pos = mul(float4(input.Position, 1.0f), view);
    pos = mul(pos, ProjectionMatrix);
    
    output.Position = pos.xyww;
    
    return output;
}