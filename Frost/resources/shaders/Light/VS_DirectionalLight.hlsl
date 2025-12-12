struct VS_Input
{
    uint VertexID : SV_VertexID;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VS_Output main(VS_Input input)
{
    VS_Output output;
    // Génération d'un triangle plein écran sans Vertex Buffer
    output.TexCoord = float2((input.VertexID << 1) & 2, (input.VertexID & 2));
    output.Position = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
    output.Position.y = -output.Position.y; // Flip Y pour DirectX
    return output;
}