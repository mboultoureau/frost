#pragma once

#include "Frost.h"

#include <memory>

class MainLayer : public Frost::Layer
{
public:
	MainLayer();

	void OnUpdate(float deltaTime) override;

private:
	std::unique_ptr<Frost::Scene> _scene;
};

