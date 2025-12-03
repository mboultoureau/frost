#pragma once

#include "Frost.h"
#include "Frost/Scene/Components/Script.h"
#include <Frost/Utils/Math/Vector.h>
#include <DirectXMath.h>

using namespace Frost;
using namespace Frost::Math;

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
    Grass(Vector3 pos, EulerAngles rot, Vector3 scale);

    void _SetClosestPlayerPosToShader();

    void UpdateShader(float dt);

private:
    GameObject _grassPlane;
    GrassMaterialParameters _params;
};

class GrassScript : public Script
{
    Grass* _grass;

public:
    GrassScript(Grass* grass) : _grass{ grass } {};
    void OnUpdate(float deltaTime) override;
};