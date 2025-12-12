#include "Samples/Rotate.h"

#include <iostream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GameLogic::Rotate::OnUpdate(float deltaTime)
    {
        auto& transform = GetGameObject().GetComponent<Transform>();
        transform.Rotate({ 0.0_deg, 1000.0_deg * deltaTime, 0.0_deg });
    }
} // namespace GameLogic
