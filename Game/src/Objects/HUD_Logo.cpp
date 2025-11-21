#include "HUD_Logo.h"
#include "../Game.h" 

using namespace Frost;
using namespace Frost::Component;

HUD_Logo::HUD_Logo()
{

	Scene& _scene = Game::GetScene();

	_hudLogo = _scene.CreateGameObject("Engine_Logo");

	Viewport viewport;
	viewport.height = 0.08;
	viewport.width = 0.15f;
	const std::string LOGO_PATH = "resources/meshes/EngineLogo.png"; 

	viewport.x = 0.85f; 
	viewport.y = 0.0f; 
	
	_hudLogo.AddComponent<HUDImage>(
		viewport,
		LOGO_PATH,
		Material::FilterMode::POINT
	);

}
