#include "HUD_Middle.h"
#include "../../Game.h"

using namespace Frost;
using namespace Frost::Component;

HUD_Middle::HUD_Middle()
{

    Scene& _scene = Game::GetScene();

    _hudLogo = _scene.CreateGameObject("Middle Image");

    Viewport viewport;
    viewport.height = 0.1;
    viewport.width = 0.1;
    const std::string LOGO_PATH = "resources/textures/bike.png";

    viewport.x = 0.8f;
    viewport.y = 0.05f;

    _hudLogo.AddComponent<HUDImage>(viewport, LOGO_PATH, Material::FilterMode::POINT);
}
