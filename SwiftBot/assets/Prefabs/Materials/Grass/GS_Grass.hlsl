#include "GrassCommon.hlsli"

#define BLADE_SEGMENTS 3
#define PI 3.14159265359

// Matrice pour aligner un vecteur "Up" (0,1,0) vers une targetNormal
float3x3 AlignToNormal(float3 targetNormal)
{
    float3 up = float3(0, 1, 0);
    if (abs(dot(up, targetNormal)) > 0.999)
        return float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    
    float3 axis = normalize(cross(up, targetNormal));
    float angle = acos(dot(up, targetNormal));
    return AngleAxis3x3(angle, axis);
}

[maxvertexcount(3 * (BLADE_SEGMENTS * 2 + 1))]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream)
{
    // On génère l'herbe pour chaque vertex du triangle tessellé
    for (int i = 0; i < 3; i++)
    {
        float3 rootPos = input[i].worldPos;
        float3 geomNormal = normalize(input[i].normal);
        
        // --- 1. BASE MATRIX (TANGENT SPACE) ---
        // On construit une base orthonormée locale sur la surface
        float3 up = geomNormal;
        
        // ASTUCE : Si la géométrie est à l'envers, 'up' pointe vers le bas.
        // Assure-toi que tes normales Mesh sont correctes. 
        // Sinon force : up = float3(0, 1, 0);
        
        float3 tangent = normalize(input[i].tangent.xyz);
        float3 binormal = normalize(cross(up, tangent) * input[i].tangent.w);
        
        // Matrice Tangent -> World (Y est UP)
        float3x3 tangentToWorld = float3x3(
            tangent.x, up.x, binormal.x,
            tangent.y, up.y, binormal.y,
            tangent.z, up.z, binormal.z
        );
        
        // --- 2. RANDOM SEED ---
        float randVal = rand(rootPos.xyz);
        
        // --- 3. ROTATION (Facing) ---
        // Rotation aléatoire autour de l'axe Y (Haut) du brin
        float3x3 facingRot = AngleAxis3x3(randVal * PI * 2.0, float3(0, 1, 0));
        
        // --- 4. VENT ---
        float windOsc = sin(Time * WindStrength + rootPos.x) * 0.5 + 0.5; // 0 à 1
        float3x3 windRot = AngleAxis3x3(windOsc * 0.5, float3(1, 0, 0)); // Penche sur X
        
        // --- 5. BENDING STATIC ---
        float3x3 bendRot = AngleAxis3x3(rand(rootPos.zyx) * 0.5, float3(1, 0, 0));

        // Dimensions
        float h = BladeHeight + (randVal * 2.0 - 1.0) * 0.2; // Variation hauteur
        float w = BladeWidth + (rand(rootPos.xzy) * 2.0 - 1.0) * 0.05;
        
        // MATRICE FINALE POUR CE BRIN
        // On applique Facing, puis on convertit en World
        float3x3 baseTransform = mul(tangentToWorld, facingRot);

        // --- GÉNÉRATION ---
        for (int j = 0; j < BLADE_SEGMENTS; j++)
        {
            float t = (float) j / BLADE_SEGMENTS;
            float tNext = (float) (j + 1) / BLADE_SEGMENTS;
            
            float currentW = w * (1.0 - t);
            float nextW = w * (1.0 - tNext);
            
            float currentH = h * t;
            float nextH = h * tNext;
            
            // Calcul de la courbure (offset sur Z local)
            float currentCurve = pow(t, BladeCurve) * BladeForward;
            float nextCurve = pow(tNext, BladeCurve) * BladeForward;
            
            // Positions LOCALES (Blade Space : Y=Haut, X=Largeur, Z=Profondeur)
            // Note le signe : X pour gauche/droite, Y pour hauteur, Z pour courbe
            float3 p1 = float3(-currentW, currentH, currentCurve);
            float3 p2 = float3(currentW, currentH, currentCurve);
            float3 p3 = float3(-nextW, nextH, nextCurve);
            float3 p4 = float3(nextW, nextH, nextCurve);
            
            // Transformation vers World Space
            // On applique le vent/bend sur la matrice ou sur les coordonnées locales
            // Pour faire simple : appliquons le vent comme une rotation locale
            
            // World Positions
            float3 wP1 = rootPos + mul(p1, baseTransform);
            float3 wP2 = rootPos + mul(p2, baseTransform);
            float3 wP3 = rootPos + mul(p3, baseTransform);
            float3 wP4 = rootPos + mul(p4, baseTransform);
            
            // Output Triangle Strip (2 triangles pour faire un quad)
            GSOutput v1, v2;
            
            // Vertex 1 (Gauche bas)
            v1.worldPos = wP1;
            v1.position = mul(float4(v1.worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
            v1.uv = float2(0, t);
            v1.normal = geomNormal; // Simplifié pour debug éclairage
            triStream.Append(v1);

            // Vertex 2 (Droite bas)
            v2.worldPos = wP2;
            v2.position = mul(float4(v2.worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
            v2.uv = float2(1, t);
            v2.normal = geomNormal;
            triStream.Append(v2);
        }
        
        // TIP VERTEX
        GSOutput vTip;
        float3 pTip = float3(0, h, pow(1.0, BladeCurve) * BladeForward);
        vTip.worldPos = rootPos + mul(pTip, baseTransform);
        vTip.position = mul(float4(vTip.worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
        vTip.uv = float2(0.5, 1);
        vTip.normal = geomNormal;
        
        triStream.Append(vTip);
        triStream.RestartStrip();
    }
}