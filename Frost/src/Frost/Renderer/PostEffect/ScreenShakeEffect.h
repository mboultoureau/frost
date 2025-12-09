#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/PostEffect.h"

namespace Frost
{
    class FROST_API ScreenShakeEffect : public PostEffect
    {
    public:
        enum class AttenuationType
        {
            Linear,
            EaseOut, // In car game, ideal for explosions or impacts
            EaseIn,  // In car game, ideal for build-up effects like engine
                     // revving
            Constant // In car game, ideal for continuous effects like road
                     // vibrations
        };

    public:
        void OnPreRender(float deltaTime, Math::Matrix4x4& viewMatrix, Math::Matrix4x4& projectionMatrix) override;
        void OnImGuiRender(float deltaTime) override;
        void Shake(float duration, float amplitude, AttenuationType type = AttenuationType::EaseOut);
        const char* GetName() const override { return "ScreenShakeEffect"; }
        virtual bool IsPostProcessingPass() const { return false; }

    private:
        float _currentTime = 0.0f;
        float _duration = 0.0f;
        float _amplitude = 0.0f;
        AttenuationType _attenuationType = AttenuationType::EaseOut;
    };
} // namespace Frost