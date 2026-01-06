// G-Buffer Pass: Pixel Shader
Texture2D AlbedoMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D MetallicMap : register(t2);
Texture2D RoughnessMap : register(t3);
Texture2D AOMap : register(t4);
Texture2D EmissionMap : register(t5);

SamplerState MaterialSampler : register(s0);

cbuffer MaterialConstants : register(b2)
{
    float2 UVTiling;
    float2 UVOffset;
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 Tangent : TEXCOORD2;
    float3 Bitangent : TEXCOORD3;
    float2 TexCoord : TEXCOORD4;
};

struct PS_Output
{
    float4 Albedo           : SV_Target0; // RGB: Albedo
    float4 Normal           : SV_Target1; // RGB: Normal
    float4 WorldPos         : SV_Target2; // RGB: World Position
    float4 Material         : SV_Target3; // R: Metalness, G: Roughness, B: Ambient Occlusion
    float4 Emission         : SV_Target4; // RGB: Emission
};

PS_Output main(PS_Input input)
{
    PS_Output output;

    float2 texCoord = input.TexCoord * UVTiling + UVOffset;

    output.Albedo.rgb = AlbedoMap.Sample(MaterialSampler, texCoord).rgb;
    output.Albedo.a = 1.0f;

    float3x3 TBN = float3x3(normalize(input.Tangent), normalize(input.Bitangent), normalize(input.Normal));

    float3 normalTangentSpace = NormalMap.Sample(MaterialSampler, texCoord).xyz * 2.0 - 1.0;
    float3 worldNormal = normalize(mul(normalTangentSpace, TBN));
    output.Normal = float4(worldNormal * 0.5f + 0.5f, 1.0f);

    output.WorldPos = float4(input.WorldPos, 1.0f);

    float metalness = MetallicMap.Sample(MaterialSampler, texCoord).b;
    float roughness = RoughnessMap.Sample(MaterialSampler, texCoord).g;
    float ao = AOMap.Sample(MaterialSampler, texCoord).r;

    output.Material = float4(metalness, roughness, ao, 1.0f);
    output.Emission = EmissionMap.Sample(MaterialSampler, texCoord);
    
    return output;
}
