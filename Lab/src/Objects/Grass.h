#pragma once

#include "Frost.h"
#include <DirectXMath.h>

using namespace Frost;

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

class Grass
{
public:
    Grass();

    void _SetClosestPlayerPosToShader();

    void Update(float dt);

private:
    GameObject _planeObject;
    GrassMaterialParameters _params;
};
