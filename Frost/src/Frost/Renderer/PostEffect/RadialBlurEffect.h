#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/PostEffect.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Utils/Math/Vector.h"

#include <memory>

namespace Frost
{
    class FROST_API RadialBlurEffect : public PostEffect
    {
    public:
        RadialBlurEffect();

        void OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination) override;
        void OnImGuiRender(float deltaTime) override;

        const char* GetName() const override { return "RadialBlurEffect"; }

        void SetCenter(const Math::Vector2& center) { _center = center; }
        void SetStrength(float strength) { _strength = strength; }
        void SetSampleCount(int sampleCount) { _sampleCount = sampleCount; }

    private:
        Math::Vector2 _center = { 0.5f, 0.5f };
        float _strength = 0.04f;
        int _sampleCount = 15;

    private:
        std::unique_ptr<Shader> _vertexShader;
        std::unique_ptr<Shader> _pixelShader;
        std::shared_ptr<Buffer> _constantsBuffer;
        std::unique_ptr<Sampler> _sampler;
    };
} // namespace Frost