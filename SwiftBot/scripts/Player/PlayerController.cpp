#include "Player/PlayerController.h"

#include <iostream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GameLogic::PlayerController::OnCreate()
    {
        /*
        _bike = GetGameObject().GetChildByName("Bike", true);
        _boat = GetGameObject().GetChildByName("Boat", true);
        _plane = GetGameObject().GetChildByName("Plane", true);

        FT_ASSERT(_bike.IsValid(), "Bike GameObject not found as child of PlayerController");
        FT_ASSERT(_boat.IsValid(), "Boat GameObject not found as child of PlayerController");
        FT_ASSERT(_plane.IsValid(), "Plane GameObject not found as child of PlayerController");
        */
    }

    void GameLogic::PlayerController::OnUpdate(float deltaTime) {}
} // namespace GameLogic
