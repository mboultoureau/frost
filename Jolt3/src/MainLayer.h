#pragma once

#include "Frost/Core/Layer.h"
#include "Frost/Core/Timer.h"
#include "Frost/Scene/Scene.h"
#include "Objects/Player.h"
#include "LabApp.h"

#include <memory>

class MainLayer : public Frost::Layer
{
public:
	MainLayer();

	void OnLateUpdate(float deltaTime) override;
	void OnAttach() override;
	void OnUpdate(float deltaTime) override;
	void OnFixedUpdate(float deltaTime) override;

private:
	std::unique_ptr<Player> _player;
};

