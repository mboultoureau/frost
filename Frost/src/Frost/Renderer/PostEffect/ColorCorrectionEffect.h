#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/PostEffect.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Asset/Texture.h"

#include <memory>
#include <string> // Nécessaire pour std::string

namespace Frost
{
    class FROST_API ColorCorrectionEffect : public PostEffect
    {
    public:
        ColorCorrectionEffect();

        void OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination) override;
        void OnImGuiRender(float deltaTime) override;

        const char* GetName() const override { return "ColorCorrectionEffect"; }

        // Setter pour une texture déjà chargée ailleurs (ex: AssetManager)
        void SetLUT(Texture* lutTexture) { _lutTexture = lutTexture; }

        // Nouvelle méthode : Charge une texture depuis le disque
        void LoadLUT(const std::string& path);

        void SetStrength(float strength) { _strength = strength; }

    private:
        float _strength = 1.0f;

        // Pointeur brut utilisé pour le rendu (rapide)
        Texture* _lutTexture = nullptr;

        // Smart pointer pour garder la texture en vie si chargée par cette classe
        std::shared_ptr<Texture> _lutResource;

        // Buffer pour ImGui (nom du fichier)
        char _lutPathBuffer[256] = "assets/textures/lut/neutral.png";

    private:
        std::unique_ptr<Shader> _vertexShader;
        std::unique_ptr<Shader> _pixelShader;
        std::shared_ptr<Buffer> _constantsBuffer;
        std::unique_ptr<Sampler> _sampler;
    };
} // namespace Frost