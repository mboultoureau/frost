#include "FreeCamera.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Component;

FreeCamera::FreeCamera()
{
    Scene& scene = Game::GetScene();

    _camera = scene.CreateGameObject("FreeCamera");
    _camera.AddComponent<Transform>(Math::Vector3{ 0.0f, 5.0f, 0.0f });
    _camera.AddComponent<Light>();
    auto& cameraComp = _camera.AddComponent<Camera>();

    // Modification directe
    cameraComp.backgroundColor.r = 0.2f;
    cameraComp.backgroundColor.g = 0.3f;
    cameraComp.backgroundColor.b = 0.4f;
    cameraComp.backgroundColor.a = 1.0f;
}
