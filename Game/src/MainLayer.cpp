#include "MainLayer.h"
#include "Game.h"
#include "Objects/Terrain.h"
#include "Objects/LevelCamera.h"
#include "Objects/LapCheckPoint.h"
#include "Objects/Wall.h"
using namespace Frost;

MainLayer::MainLayer() : Layer("MainLayer")
{
}

void MainLayer::OnAttach()
{
	Scene& _scene = Game::GetScene();
	_terrain = std::make_unique<Terrain>(Terrain());
	//_wall = std::make_unique<Wall>(Wall());

	_gamestate = GameState();
	_gamestate.SetLap(2);

	_terrain = std::make_unique<Terrain>();
	_sun = std::make_unique<Sun>();

	_player = std::make_unique<Player>();

	_checkPoint1 = std::make_shared<LapCheckPoint>(Vector3{ -365, 75, -32 },_gamestate);
	_checkPoint2 = std::make_shared<CheckPoint>(Vector3{ -230, 75, 239 });
	_checkPoint3 = std::make_shared<CheckPoint>(Vector3{ -130, 75, 180 });
	_checkPoint4 = std::make_shared<CheckPoint>(Vector3{ -147, 75, -82 });
	_checkPoint5 = std::make_shared<CheckPoint>(Vector3{ 190, 75, -524 });


	//link 1 / 2
	_checkPoint1->AddChild(_checkPoint2);
	_checkPoint2->AddParent(_checkPoint1);
	//link 1 / 3
	_checkPoint1->AddChild(_checkPoint3);
	_checkPoint3->AddParent(_checkPoint1);

	//DEBUG
		//link 2 / 1
/*		_checkPoint2->AddChild(_checkPoint1);
		_checkPoint1->AddParent(_checkPoint2);
*/
	//link 2 / 4
	_checkPoint2->AddChild(_checkPoint4);
	_checkPoint4->AddParent(_checkPoint2);
	//link 3 / 4
	_checkPoint3->AddChild(_checkPoint4);
	_checkPoint4->AddParent(_checkPoint3);

	//link 4 / 5
	_checkPoint4->AddChild(_checkPoint5);
	_checkPoint5->AddParent(_checkPoint4);

	//link 5 / 1
	_checkPoint5->AddChild(_checkPoint1);
	_checkPoint1->AddParent(_checkPoint5);

	_checkPoint1->ActivatePhysics();

	CheckPoint::lastCheckPoint = _checkPoint1->GetGameObjectId();

	logo = HUD_Logo();

	LevelCamera levelCamera;

	_pauseHandlerUUID = EventManager::Subscribe<Frost::PauseEvent>(
		FROST_BIND_EVENT_FN(MainLayer::OnGamePaused));

	_unpauseHandlerUUID = EventManager::Subscribe<Frost::UnPauseEvent>(
		FROST_BIND_EVENT_FN(MainLayer::OnGameUnpaused));
}

void MainLayer::OnUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_scene.Update(deltaTime);
}

void MainLayer::OnLateUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();
	_scene.LateUpdate(deltaTime);
}

void MainLayer::OnFixedUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();
	if (_gamestate.Finished()) {

		auto winScreen = _scene.CreateGameObject("victoryScreen");

		Viewport viewport;
		viewport.height = 0.2;
		viewport.width = 0.4f;
		const std::string WIN_PATH = "resources/textures/victoryScreen.png";

		viewport.x = 0.25f;
		viewport.y = 0.4;
		_gamestate.ResetLap();
		_scene.AddComponent<HUDImage>(
			winScreen,
			viewport,
			WIN_PATH,
			Material::FilterMode::POINT
		);
	}
	else {
		_player->FixedUpdate(deltaTime);
		_checkPoint1->FixedUpdate(deltaTime);
		_checkPoint2->FixedUpdate(deltaTime);
		_checkPoint3->FixedUpdate(deltaTime);
		_checkPoint4->FixedUpdate(deltaTime);
	}
	_scene.FixedUpdate(deltaTime);
	
}

void MainLayer::OnDetach()
{
	EventManager::Unsubscribe<PauseEvent>(_pauseHandlerUUID);
	EventManager::Unsubscribe<UnPauseEvent>(_unpauseHandlerUUID);
}

bool MainLayer::OnGamePaused(Frost::PauseEvent& e)
{
	_paused = true;
	return true;
}

bool MainLayer::OnGameUnpaused(Frost::UnPauseEvent& e)
{
	_paused = false;
	return true;
}