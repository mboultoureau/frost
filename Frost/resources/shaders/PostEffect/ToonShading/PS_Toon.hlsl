Texture2D SourceTexture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);

#define MAX_STEPS 8

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer ToonConstants : register(b0)
{
    int redStepCount; // Nombre de paliers pour le rouge (1-16)
    int greenStepCount; // Nombre de paliers pour le vert (1-16)
    int blueStepCount; // Nombre de paliers pour le bleu (1-16)
    int pad0;
    
    float redSteps[MAX_STEPS]; // Valeurs des paliers rouge [0.0 - 1.0]
    float greenSteps[MAX_STEPS]; // Valeurs des paliers vert [0.0 - 1.0]
    float blueSteps[MAX_STEPS]; // Valeurs des paliers bleu [0.0 - 1.0]
    
    float3 pad1;
    
    // Paramètres de détection de contours
    float edgeThreshold;        // Seuil normales (ex: 0.3 à 0.6)
    float edgeStrength;         // Intensité du contour (ex: 1.0)
    float depthThreshold;       // Seuil profondeur (ex: 0.01 à 0.1)
    float depthSensitivity;     // Sensibilité profondeur (ex: 50.0)
    
    // Paramètres de shading basé sur les normales
    float normalStrength;       // Multiplicateur pour l'assombrissement (ex: 1.0)
    float normalMin;            // Valeur minimale de normal (ex: 0.0)
    float normalMax;            // Valeur maximale de normal (ex: 1.0)
    
    float3 edgeColor;           // Couleur des contours (ex: noir = 0,0,0)
    
    float2 texelSize;
};

float QuantizeToClosest(float value, float steps[MAX_STEPS], int stepCount)
{
    float closest = steps[0];
    float minDist = abs(value - closest);
        
    for (int i = 1; i < stepCount; i++)
    {
        float dist = abs(value - steps[i]);
        if (dist < minDist)
        {
            minDist = dist;
            closest = steps[i];
        }
    }
    
    return closest;
}

float4 main(PS_Input input) : SV_Target
{
    // ===== 1. QUANTIFICATION DES COULEURS (TOON SHADING) =====
    float4 currentColor = SourceTexture.Sample(Sampler, input.TexCoord);
    float3 quantizedColor;
    
    quantizedColor.r = QuantizeToClosest(currentColor.r, redSteps, redStepCount);
    quantizedColor.g = QuantizeToClosest(currentColor.g, greenSteps, greenStepCount);
    quantizedColor.b = QuantizeToClosest(currentColor.b, blueSteps, blueStepCount);
    
    // ===== 2. DÉTECTION DES CONTOURS PAR NORMALES =====
    float3 normalCenter = NormalTexture.Sample(Sampler, input.TexCoord).xyz;
    normalCenter = normalize(normalCenter * 2.0 - 1.0);
    
    float3 normalTop = NormalTexture.Sample(Sampler, input.TexCoord + float2(0, -texelSize.y)).xyz;
    float3 normalBottom = NormalTexture.Sample(Sampler, input.TexCoord + float2(0, texelSize.y)).xyz;
    float3 normalLeft = NormalTexture.Sample(Sampler, input.TexCoord + float2(-texelSize.x, 0)).xyz;
    float3 normalRight = NormalTexture.Sample(Sampler, input.TexCoord + float2(texelSize.x, 0)).xyz;

    
    normalTop = normalize(normalTop * 2.0 - 1.0);
    normalBottom = normalize(normalBottom * 2.0 - 1.0);
    normalLeft = normalize(normalLeft * 2.0 - 1.0);
    normalRight = normalize(normalRight * 2.0 - 1.0);
    
    float edgeX = dot(normalRight - normalLeft, normalRight - normalLeft);
    float edgeY = dot(normalTop - normalBottom, normalTop - normalBottom);
    float normalEdge = sqrt(edgeX + edgeY);
    
    // ===== 3. COMBINAISON DES DÉTECTIONS =====
    normalEdge = smoothstep(edgeThreshold, edgeThreshold + 0.1, normalEdge);    
    float edge = saturate(normalEdge * edgeStrength);
    
    // ===== 4. APPLICATION DES CONTOURS =====
    // Interpoler entre la couleur shadée et la couleur de contour
    float3 finalColor = lerp(quantizedColor, edgeColor, edge);
    
    return float4(finalColor, currentColor.a);
}