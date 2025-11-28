#include "GrassCommon.hlsli"

#define BLADE_SEGMENTS 3
#define PI 3.14159265359

GSOutput GenerateGrassVertex(float3 vertexPosition, float width, float height,
                            float forward, float2 uv, float3x3 transformMatrix)
{
    GSOutput output;
    
    // Position en tangent space
    float3 tangentPoint = float3(width, forward, height);
    
    // Transformation vers world space
    float3 localPosition = vertexPosition + mul(tangentPoint, transformMatrix);
    
    float4 worldPos = mul(float4(localPosition, 1.0), WorldMatrix);
    output.worldPos = worldPos.xyz;
    output.position = mul(worldPos, mul(ViewMatrix, ProjectionMatrix));
    
    // Normal en tangent space puis world space
    float3 tangentNormal = normalize(float3(0, -1, forward));
    float3 localNormal = mul(tangentNormal, transformMatrix);
    output.normal = normalize(mul(float4(localNormal, 0.0), WorldMatrix).xyz);
    
    output.uv = uv;
    
    return output;
}

[maxvertexcount(3 * (BLADE_SEGMENTS * 2 + 1))]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream)
{
    // Générer un blade d'herbe pour chaque vertex du triangle
    for (int i = 0; i < 3; i++)
    {
        float3 pos = input[i].position;
        float3 vNormal = input[i].normal;
        float4 vTangent = input[i].tangent;
        float3 vBinormal = cross(vNormal, vTangent.xyz) * vTangent.w;
    
        // Construction de la matrice tangent-to-local
        float3x3 tangentToLocal = float3x3(
            vTangent.x, vBinormal.x, vNormal.x,
            vTangent.y, vBinormal.y, vNormal.y,
            vTangent.z, vBinormal.z, vNormal.z
        );
    
        // Rotation aléatoire autour de la normale (axe vertical de la surface)
        float3x3 facingRotationMatrix = AngleAxis3x3(
            rand(pos) * 2.0 * PI,
            normalize(mul(float4(vNormal, 0.0), WorldMatrix).xyz)
        );
    
        // Courbure aléatoire vers l'avant (en tangent space)
        float3x3 bendRotationMatrix = AngleAxis3x3(
            rand(pos.zzx) * BendRotationRandom * PI * 0.4,
            float3(-1, 0, 0)
        );
        
        // Calcul du vent
        float wind1 = sin(Time * 1.4 + pos.x * 0.3 + pos.z * 0.5) * 0.6;
        float wind2 = sin(Time * 2.2 + pos.x * 1.1) * 0.25;
        float wind = (wind1 + wind2) * WindStrength;
        
        float3x3 windRotation = AngleAxis3x3(wind, float3(0, 0, 1));

        float3x3 transformationMatrix = tangentToLocal;
    
        // Dimensions aléatoires
        float height = (rand(pos.zyx) * 2.0 - 1.0) * BladeHeightRandom + BladeHeight;
        float width = (rand(pos.xzy) * 2.0 - 1.0) * BladeWidthRandom + BladeWidth;
        float forward = rand(pos.yyz) * BladeForward;
    
        // Génération des segments
        for (int j = 0; j < BLADE_SEGMENTS; j++)
        {
            float t = j / (float) BLADE_SEGMENTS;
            float segmentHeight = height * t;
            float segmentWidth = width * (1.0 - t);
            float segmentForward = pow(t, BladeCurve) * forward;
        
            // Générer les deux vertices du segment
            float3 tangentLeft = float3(segmentWidth, segmentForward, segmentHeight);
            float3 tangentRight = float3(-segmentWidth, segmentForward, segmentHeight);
            
            float3 localLeft = mul(tangentLeft, transformationMatrix);
            float3 localRight = mul(tangentRight, transformationMatrix);
            
            // Appliquer la rotation "facing" en world space
            float3 worldLeft = mul(float4(localLeft, 0.0), WorldMatrix).xyz;
            float3 worldRight = mul(float4(localRight, 0.0), WorldMatrix).xyz;
            
            worldLeft = mul(worldLeft, facingRotationMatrix);
            worldRight = mul(worldRight, facingRotationMatrix);
            
            // Reconstruire les outputs
            GSOutput outLeft, outRight;
            
            outLeft.worldPos = mul(float4(pos, 1.0), WorldMatrix).xyz + worldLeft;
            outLeft.position = mul(float4(outLeft.worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
            outLeft.uv = float2(0, t);
            
            outRight.worldPos = mul(float4(pos, 1.0), WorldMatrix).xyz + worldRight;
            outRight.position = mul(float4(outRight.worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
            outRight.uv = float2(1, t);
            
            // Calcul des normales
            float3 tangentNormal = normalize(float3(0, -1, segmentForward));
            float3 localNormal = mul(tangentNormal, transformationMatrix);
            float3 worldNormal = normalize(mul(float4(localNormal, 0.0), WorldMatrix).xyz);
            worldNormal = mul(worldNormal, facingRotationMatrix);
            
            worldNormal = normalize(lerp(worldNormal, vNormal, 0.3));
            
            // Orienter la normale vers la caméra pour éclairage bidirectionnel
            float3 viewDir = normalize(CameraPosition - outLeft.worldPos);
            if (dot(worldNormal, viewDir) < 0)
            {
                worldNormal = -worldNormal;
            }
            
            outLeft.normal = worldNormal;
            outRight.normal = worldNormal;
            
            triStream.Append(outLeft);
            triStream.Append(outRight);
            
            transformationMatrix = mul(mul(tangentToLocal, windRotation), bendRotationMatrix);
        }
    
        // Vertex au sommet
        float3 tangentTip = float3(0, forward, height);
        float3 localTip = mul(tangentTip, transformationMatrix);
        float3 worldTip = mul(float4(localTip, 0.0), WorldMatrix).xyz;
        worldTip = mul(worldTip, facingRotationMatrix);
        
        GSOutput outTip;
        outTip.worldPos = mul(float4(pos, 1.0), WorldMatrix).xyz + worldTip;
        outTip.position = mul(float4(outTip.worldPos, 1.0), mul(ViewMatrix, ProjectionMatrix));
        outTip.uv = float2(0.5, 1);
        
        float3 tangentNormal = normalize(float3(0, -1, forward));
        float3 localNormal = mul(tangentNormal, transformationMatrix);
        float3 worldNormal = normalize(mul(float4(localNormal, 0.0), WorldMatrix).xyz);
        outTip.normal = mul(worldNormal, facingRotationMatrix);
        
        outTip.normal = normalize(lerp(outTip.normal, vNormal, 0.3));
        
        // Orienter la normale du sommet vers la caméra
        float3 viewDir = normalize(CameraPosition - outTip.worldPos);
        if (dot(outTip.normal, viewDir) < 0)
        {
            outTip.normal = -outTip.normal;
        }
        
        triStream.Append(outTip);
        triStream.RestartStrip();
    }
}