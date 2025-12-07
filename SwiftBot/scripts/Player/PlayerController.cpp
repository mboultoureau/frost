#include "Player/PlayerController.h"

#include <iostream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GameLogic::PlayerController::OnCreate()
    {
        std::cout << "Hello" << std::endl;
        // FT_INFO("Script PlayerController initialized");
    }

    void GameLogic::PlayerController::OnUpdate(float deltaTime)
    {
        std::cout << deltaTime << std::endl;
        // FT_INFO("Script PlayerController updated {}ms", deltaTime);
    }
} // namespace GameLogic
