Texture2D SourceTexture : register(t0);
Texture2D LutTexture : register(t1); // La texture de LUT (ex: 256x16)
SamplerState SourceSampler : register(s0);
struct PS_Input
{
float4 Position : SV_POSITION;
float2 TexCoord : TEXCOORD0;
};
cbuffer ColorCorrectionConstants : register(b0)
{
float Strength;
float3 Padding;
};
// Parametres pour une LUT standard de 256x16 (16 tranches de 16x16)
static const float COLORS = 16.0;
static const float MAXCOLOR = 15.0;
float4 main(PS_Input input) : SV_TARGET
{
float4 sceneColor = SourceTexture.Sample(SourceSampler, input.TexCoord);

// Calcul de la position dans la LUT
// On utilise le canal Bleu pour determiner la tranche (Z)
float cell = sceneColor.b * MAXCOLOR;

float cell_l = floor(cell); // Tranche inferieure
float cell_h = ceil(cell);  // Tranche superieure

float halfPixelX = 0.5 / 256.0; // Correction demi-pixel pour eviter le bleeding
float halfPixelY = 0.5 / 16.0;

// Calcul des coordonnees UV dans la tranche inferieure
float rOffset = halfPixelX + sceneColor.r / COLORS * (MAXCOLOR / COLORS);
float gOffset = halfPixelY + sceneColor.g * (MAXCOLOR / COLORS);

float2 lutPosL;
lutPosL.x = cell_l / COLORS + rOffset;
lutPosL.y = gOffset;

// Calcul des coordonnees UV dans la tranche superieure
float2 lutPosH;
lutPosH.x = cell_h / COLORS + rOffset;
lutPosH.y = gOffset;

// Echantillonnage
float4 gradedColorL = LutTexture.Sample(SourceSampler, lutPosL);
float4 gradedColorH = LutTexture.Sample(SourceSampler, lutPosH);

// Interpolation entre les deux tranches selon la partie fractionnaire du Bleu
float4 gradedColor = lerp(gradedColorL, gradedColorH, frac(cell));

// Melange final avec la force de l'effet
return lerp(sceneColor, gradedColor, Strength);
}