#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/PostEffect.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Sampler.h"

#define MAX_STEPS 8

namespace Frost
{
    struct alignas(16) ToonConstants
    {
        // Parametres de quantification des couleurs
        int redStepCount;
        int greenStepCount;
        int blueStepCount;
        int pad0 = 0;

        DirectX::XMFLOAT4 redSteps[MAX_STEPS];   // 8 * 16 bytes
        DirectX::XMFLOAT4 greenSteps[MAX_STEPS]; // 8 * 16 bytes
        DirectX::XMFLOAT4 blueSteps[MAX_STEPS];  // 8 * 16 bytes

        // Parametres de detection de contours
        float edgeThreshold;    // Seuil normales (ex: 0.3 a 0.6)
        float edgeStrength;     // Intensita du contour (ex: 1.0)
        float depthThreshold;   // Seuil profondeur (ex: 0.01 a 0.1)
        float depthSensitivity; // Sensibilite profondeur (ex: 50.0)

        // Parametres de shading base sur les normales
        float normalStrength; // Multiplicateur pour l'assombrissement (ex: 1.0)
        float normalMin;      // Valeur minimale de normal (ex: 0.0)
        float normalMax;      // Valeur maximale de normal (ex: 1.0)

        DirectX::XMVECTOR edgeColor; // Couleur des contours (ex: noir = 0,0,0)

        DirectX::XMFLOAT2 texelSize;
    };

    class FROST_API ToonEffect : public PostEffect
    {
    public:
        ToonEffect();

        void OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination) override;
        void OnImGuiRender(float deltaTime) override;

        const char* GetName() const override { return "ToonEffect"; }

    private:
        // ImGui control
        int _activeCenter = 0;

    private:
        std::unique_ptr<Shader> _vertexShader;
        std::unique_ptr<Shader> _pixelShader;
        std::shared_ptr<Buffer> _constantsBuffer;
        std::unique_ptr<Sampler> _sampler;
        ToonConstants constants;
    };
} // namespace Frost