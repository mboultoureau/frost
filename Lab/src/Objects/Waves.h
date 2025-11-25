#pragma once

#include "Frost.h"

using namespace Frost;

struct alignas(16) WaveMaterialParameters
{
    float Time = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    float TessellationFactor = 16.0f;
    //float Padding[4];
};

class Waves
{
public:
    Waves();

    void Update(float dt);

private:
    GameObject _planeObject;
    WaveMaterialParameters _params;
};
