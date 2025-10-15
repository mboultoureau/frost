#pragma once

#include "Frost/Core/Layer.h"
#include "Frost/Scene/Scene.h"
#include "LabApp.h"

#include <memory>

class MainLayer : public Frost::Layer
{
public:
	MainLayer();

	void OnUpdate(float deltaTime) override;

private:
};

