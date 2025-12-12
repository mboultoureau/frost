#include "BoostCommon.hlsli"

struct PS_Output
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Material : SV_Target3;
};

PS_Output main(VS_Output input)
{
    PS_Output output;
    
    // Projection dans le repère local
    float3 localPos = input.LocalPos * 5;
    
    // Animation: déplacement le long de Y
    float scroll = Frequency * localPos.z -Time * 2.0f;
    
    // Créer des bandes qui se répètent
    float band = frac(scroll);
    
    // Calculer la distance dans le plan perpendiculaire
    float distanceFromCenter = (abs(localPos.x)) * Frequency;
    
    // Déterminer dans quelle "zone" on se trouve
    float zoneIndex = floor(distanceFromCenter);
    bool isEvenZone = (fmod(zoneIndex, 2.0f) < 0.5f);
    
    // La forme du chevron dans cette zone
    float chevronShape = frac(distanceFromCenter);
    
    // Inverser la forme pour les zones impaires pour créer le zigzag
    if (!isEvenZone)
    {
        chevronShape = 1.0f - chevronShape;
    }
    
    // Le masque du triangle/chevron
    float triangleMask = step(chevronShape, band);
    
    // Alternance des couleurs basée sur l'index de la bande
    float bandIndex = floor(scroll);
    float colorSwitch = frac(bandIndex * 0.5f);
    
    // Couleurs
    float3 Color1 = float3(0.2f, .0f, 0.2f); // Jaune
    float3 Color2 = float3(.15f, .0f, .25f); // Rouge
    
    // Alterne entre jaune et rouge
    float3 activeColor = lerp(Color1, Color2, step(0.5f, colorSwitch));
    
    // Couleur de base sombre quand pas dans un triangle
    float3 baseColor = float3(1.f, 1.f, 0.5f);
    
    // Application du masque triangulaire
    float3 boostColor = lerp(baseColor, activeColor, triangleMask);
    
    // Ajout d'un effet de pulsation
    float pulse = 0.9f + 0.1f * sin(Time * 5.0f);
    boostColor *= pulse;
    
    output.Albedo = float4(boostColor, 1.0f);
    output.Normal = float4(normalize(input.Normal) * 0.5f + 0.5f, 1.0f);
    output.WorldPos = float4(input.WorldPos, 1.0f);
    output.Material = float4(0.0f, 0.1f, 1.0f, 0.0f);
    
    return output;
}