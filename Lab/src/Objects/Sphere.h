#pragma once

#include "Frost.h"

#include <Jolt/Jolt.h>

class Sphere
{

public:
    Sphere();

private:
    Frost::GameObject _sphere;
    JPH::Body* _bodyId;

private:
    void _SetupPhysics();
};
