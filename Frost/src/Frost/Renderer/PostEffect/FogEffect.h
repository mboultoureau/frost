#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/PostEffect.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Sampler.h"

#include <memory>

namespace Frost
{
    class FROST_API FogEffect : public PostEffect
    {
    public:
        FogEffect();

        void OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination) override;
        void OnImGuiRender(float deltaTime) override;

        const char* GetName() const override { return "FogEffect"; }

        void SetFog(float minDepth, float strength, Frost::Math::Vector3 color)
        {
            SetMinDepth(minDepth);
            SetStrength(strength);
            SetColor(color);
        }
        void SetMinDepth(float minDepth) { _minDepth = minDepth; }
        void SetStrength(float strength) { _strength = strength; }
        void SetColor(Frost::Math::Vector3 color)
        {
            _red = color.r;
            _green = color.g;
            _blue = color.b;
        }

    private:
        float _maxDepth = 0.999f;
        float _minDepth = 0.85f;
        float _strength = 0.85f;
        float _red = 0.f;
        float _green = 0.3f;
        float _blue = 1.f;

        // ImGui control
        int _activeCenter = 0;

    private:
        std::unique_ptr<Shader> _vertexShader;
        std::unique_ptr<Shader> _pixelShader;
        std::shared_ptr<Buffer> _constantsBuffer;
        std::unique_ptr<Sampler> _sampler;
    };
} // namespace Frost