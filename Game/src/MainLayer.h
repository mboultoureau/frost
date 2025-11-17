#pragma once

#include "Frost.h"
#include "Frost/Core/Layer.h"
#include "Frost/Core/Timer.h"
#include "Frost/Scene/Scene.h"
#include "Objects/Player.h"
#include "Objects/CheckPoint.h"
#include "Objects/GameState.h"
#include "Objects/HUD_Logo.h"
#include "Objects/Sun.h"
#include "LabApp.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"

#include <memory>
#include "Objects/Terrain.h"
#include "Objects/Wall.h"

class MainLayer : public Frost::Layer
{
public:
	MainLayer();

	void OnLateUpdate(float deltaTime) override;
	void OnAttach() override;
	void OnUpdate(float deltaTime) override;
	void OnFixedUpdate(float deltaTime) override;
	void OnDetach() override;

private:
	bool OnGamePaused(Frost::PauseEvent& e);
	bool OnGameUnpaused(Frost::UnPauseEvent& e);

	GameState _gamestate;

	EventHandlerId _pauseHandlerUUID;
	EventHandlerId _unpauseHandlerUUID;
	
	std::unique_ptr<Terrain> _terrain;
	std::unique_ptr<Sun> _sun;
	std::unique_ptr<Player> _player;
	std::shared_ptr<CheckPoint> _checkPoint1;
	std::shared_ptr<CheckPoint> _checkPoint2;
	std::shared_ptr<CheckPoint> _checkPoint3;
	std::shared_ptr<CheckPoint> _checkPoint4;
	std::shared_ptr<CheckPoint> _checkPoint5;
	std::shared_ptr<Wall> _wall;
	HUD_Logo logo;
};

