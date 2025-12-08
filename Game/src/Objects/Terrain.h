#pragma once

#include "Frost.h"

using namespace Frost;
using namespace Frost::Math;

class Terrain
{
public:
    Terrain();

private:
    GameObject _terrain;

    void MakeTerrain(Frost::Math::Vector3 pos, std::string heightmap, std::string texturePath);
    void MakeCube(Vector3 pos, EulerAngles rot, Vector3 scale, std::string texturePath, bool isPhysics);
};
