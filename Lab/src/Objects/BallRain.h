#pragma once

#include "Frost.h"

using namespace Frost;

class BallRain
{
    int scale = 1000;
    int dist = 50;

public:
    BallRain();

private:
    void InstantiateDroplet(float x, float y);
};
