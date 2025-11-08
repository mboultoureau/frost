#pragma once

#include "Frost.h"
#include "Frost/Core/Layer.h"
#include "Frost/Core/Timer.h"
#include "Frost/Scene/Scene.h"
#include "Objects/Player.h"
#include "LabApp.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"

#include <memory>

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

	Frost::UUID _pauseHandlerUUID;
	Frost::UUID _unpauseHandlerUUID;
	std::unique_ptr<Player> _player;
};

