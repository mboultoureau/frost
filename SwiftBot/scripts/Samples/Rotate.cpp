#include "Samples/Rotate.h"

#include <iostream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GameLogic::Rotate::OnCreate()
    {
        std::cout << "Hello" << std::endl;
        // FT_INFO("Script PlayerController initialized");
    }

    void GameLogic::Rotate::OnUpdate(float deltaTime)
    {
        std::cout << deltaTime << std::endl;
        std::cout << "deltaTime" << std::endl;

        auto& transform = GetGameObject().GetComponent<Transform>();
        transform.Rotate({ 0.0_deg, 1000.0_deg * deltaTime, 0.0_deg });

        // FT_INFO("Script PlayerController updated {}ms", deltaTime);
    }
} // namespace GameLogic
