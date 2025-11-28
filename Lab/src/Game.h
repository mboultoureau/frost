#pragma once

#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include "Frost/Scene/Components/Script.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Utils/NoCopy.h"

#include <memory>
// #include <MainLayer.h>

class MainLayer;
class Lab;

class Game : Frost::NoCopy
{
public:
    Game(Lab* app);

    static Game& Get();
    static Frost::Scene& GetScene() { return *Get()._scene; }
    static MainLayer* GetMainLayer() { return Get()._mainLayer; }

private:
    std::unique_ptr<Frost::Scene> _scene;
    MainLayer* _mainLayer = nullptr;
    Lab* _app;
    bool OnGameReset(Frost::ResetEvent& e);
    void InitGame();

    static Game* _singleton;
};
