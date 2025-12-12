#pragma once

#include <Frost.h>

namespace GameLogic
{
    struct alignas(16) BoostMaterialParameters
    {
        float Time = 0.0f;
        float Amplitude = 0.1f;
        float Frequency = 1.0f;
        float padding = 0;
    };

    class Boost : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;
        void OnCollisionStay(Frost::BodyOnContactParameters params, float deltaTime) override;

    private:
        BoostMaterialParameters _shaderParams;
        float _waveAmplitude = 0.2f;
        float _waveFrequency = 1.0f;
        Frost::Math::Vector3 _direction{ 0.0f, 1.0f, 1.0f };
        float _impulse = 5.0f;
    };
} // namespace GameLogic
