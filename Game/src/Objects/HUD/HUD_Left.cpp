#include "HUD_Left.h"
#include "../../Game.h"

using namespace Frost;
using namespace Frost::Component;

HUD_Left::HUD_Left()
{

    Scene& _scene = Game::GetScene();

    _hudLogo = _scene.CreateGameObject("Left Image");

    Viewport viewport;
    viewport.height = 0.05;
    viewport.width = 0.05;
    const std::string LOGO_PATH = "resources/textures/plane.png";

    viewport.x = 0.75f;
    viewport.y = 0.1f;

    _hudLogo.AddComponent<HUDImage>(viewport, LOGO_PATH, Material::FilterMode::POINT);
}
