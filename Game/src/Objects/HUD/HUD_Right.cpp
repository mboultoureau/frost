#include "HUD_Right.h"
#include "../../Game.h"

using namespace Frost;
using namespace Frost::Component;

HUD_Right::HUD_Right()
{

    Scene& _scene = Game::GetScene();

    _hudLogo = _scene.CreateGameObject("Right Image");

    Viewport viewport;
    viewport.height = 0.05;
    viewport.width = 0.05;
    const std::string LOGO_PATH = "resources/textures/boat.png";

    viewport.x = 0.9f;
    viewport.y = 0.1f;

    _hudLogo.AddComponent<HUDImage>(viewport, LOGO_PATH, Material::FilterMode::POINT);
}
