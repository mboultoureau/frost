struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};


float4 main(PS_Input input) : SV_TARGET
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    return float4(finalColor, 1.0f);
}