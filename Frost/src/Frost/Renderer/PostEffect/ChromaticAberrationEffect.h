#pragma once

#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/PostEffect.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Utils/Math/Vector.h"

#include <memory>

namespace Frost
{
    class ChromaticAberrationEffect : public PostEffect
    {
    public:
        ChromaticAberrationEffect();

        void OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination) override;
        void OnImGuiRender(float deltaTime) override;

        const char* GetName() const override { return "ChromaticAberrationEffect"; }

        void SetCenterRed(const Math::Vector2& center) { _centerRed = center; }
        void SetCenterGreen(const Math::Vector2& center) { _centerGreen = center; }
        void SetCenterBlue(const Math::Vector2& center) { _centerBlue = center; }
        void SetStrength(float strength) { _strength = strength; }

    private:
        Math::Vector2 _centerRed = { 0.5f, 0.5f };
        Math::Vector2 _centerGreen = { 0.5f, 0.5f };
        Math::Vector2 _centerBlue = { 0.5f, 0.5f };
        float _strength = 0.02f;

        // ImGui control
        int _activeCenter = 0;

    private:
        std::unique_ptr<Shader> _vertexShader;
        std::unique_ptr<Shader> _pixelShader;
        std::shared_ptr<Buffer> _constantsBuffer;
        std::unique_ptr<Sampler> _sampler;
    };
} // namespace Frost