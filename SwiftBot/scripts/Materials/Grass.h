#pragma once

#include <Frost.h>
#include <DirectXMath.h>

namespace GameLogic
{
    struct alignas(16) GrassMaterialParameters
    {
        DirectX::XMFLOAT4 TopColor;
        DirectX::XMFLOAT4 BottomColor;

        float Time;
        float BladeWidth;
        float BladeWidthRandom;
        float BladeHeight;

        float BladeHeightRandom;
        float BladeForward;
        float BladeCurve;
        float BendRotationRandom;

        float TessellationFactor;
        float WindStrength;
        float Padding[2];
    };

    class Grass : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;

    private:
        GrassMaterialParameters _params;
        float _chunkSize = 20.0f;
        int _gridSize = 10;

        void _CreateGrassChunk(Frost::Math::Vector3 position);
    };
} // namespace GameLogic