#pragma once

#include <Frost.h>
#include <Frost/Utils/Math/Vector.h>
#include <DirectXMath.h>

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

    DirectX::XMVECTOR CameraPosition;
};

namespace GameLogic
{

    class GrassScript : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;

    private:
        void _SetClosestPlayerPosToShader();
        void _UpdateMaterialParameters();

    private:
        GrassMaterialParameters _params;
    };
} // namespace GameLogic
