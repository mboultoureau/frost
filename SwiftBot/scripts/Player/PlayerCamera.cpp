#include "Player/PlayerCamera.h"
#include "Physics/PhysicLayer.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void PlayerCamera::OnCreate()
    {
        // Post effects
        auto camera = GetGameObject().GetChildByName("Camera");
        FT_ASSERT(camera.IsValid(), "PlayerCamera script requires a child GameObject named 'Camera'");

        auto& camComponent = camera.GetComponent<Camera>();
        camComponent.postEffects.push_back(std::make_shared<ToonEffect>());
        camComponent.postEffects.push_back(std::make_shared<FogEffect>());
        camComponent.postEffects.push_back(std::make_shared<RadialBlurEffect>());
        camComponent.postEffects.push_back(std::make_shared<ScreenShakeEffect>());
    }
} // namespace GameLogic