#pragma once

#include "Frost.h"

using namespace Frost;

class Terrain
{
public:
    Terrain();

private:
    GameObject _terrain;

    void MakeTerrain(Frost::Math::Vector3 pos, std::string heightmap, std::string texturePath);
    void MakeCube(Frost::Math::Vector3 pos,
                  Frost::Math::EulerAngles rot,
                  Frost::Math::Vector3 scale,
                  std::string texturePath,
                  bool isPhysics);
};
