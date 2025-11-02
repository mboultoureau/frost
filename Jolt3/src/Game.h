#pragma once

#include "Frost/Scene/Scene.h"
#include "Frost/Utils/NoCopy.h"

#include <memory>

class Lab;

class Game : Frost::NoCopy
{
public:
	Game(Lab* app);

	static Game& Get();
	static Frost::Scene& GetScene() { return *Get()._scene; }

private:
	std::unique_ptr<Frost::Scene> _scene;
	Lab* _app;

	static Game* _singleton;
};

