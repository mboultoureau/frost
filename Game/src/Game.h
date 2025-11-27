#pragma once

#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Utils/NoCopy.h"
#include <memory>

class Lab;
class MainLayer;

class Game : Frost::NoCopy
{
public:
    Game(Lab* app);

    static Game& Get();
    static Frost::Scene& GetScene() { return *Get()._scene; }
    static MainLayer* GetMainLayer() { return Get()._mainLayer; }

private:
    std::unique_ptr<Frost::Scene> _scene;
    Lab* _app;

    MainLayer* _mainLayer = nullptr;
    bool OnGameReset(Frost::ResetEvent& e);
    void InitGame();
    static Game* _singleton;
};
