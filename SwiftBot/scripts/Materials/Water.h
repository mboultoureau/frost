#pragma once

#include <Frost.h>

namespace GameLogic
{
    struct alignas(16) WaterMaterialParameters
    {
        float Time = 0.0f;
        float Padding1[3];

        float TopAmplitude = 1.0f;
        float TopFrequency = 1.0f;
        float TopWaveLength = 0.1f;

        float SideAmplitude = 0.1f;
        float SideFrequency = 1.0f;

        float TessellationFactor = 16.0f;
        float BevelSize = 0.1f;
    };

    class Water : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;
        void OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) override;
        void OnCollisionStay(Frost::BodyOnContactParameters params, float deltaTime) override;

    private:
        WaterMaterialParameters _shaderParams;
        float _maxWaterHeight = 2.0f;
        float _minWaterHeight = 0.0f;
        float _waveFrequency = 1.0f;
        float _waveLength = .1f;
        float _waveAmplitude = 0.2f;
    };
} // namespace GameLogic
