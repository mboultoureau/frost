#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>

using namespace Frost;

class Player
{
public:
    Player();
    void FixedUpdate(float deltaTime);
    GameObject GetPlayerID() { return _playerController; };

private:
    GameObject _playerController;
    GameObject _camera;
    void InitializePhysics();
};
