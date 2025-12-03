#pragma once

#include "Frost.h"
#include "Frost/Core/Layer.h"
#include "Frost/Core/Timer.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Scene/Scene.h"
#include "LabApp.h"
#include "Objects/CheckPoint.h"
#include "Objects/GameState.h"
#include "Objects/HUD/SelectedImageManager.h"

#include "Objects/Player/Player.h"
#include "Objects/Portal.h"
#include "Objects/Sun.h"

#include "Objects/Terrain.h"
#include "Objects/Wall.h"
#include "Objects/Water.h"
#include "Objects/Grass.h"
#include <memory>

class MainLayer : public Frost::Layer
{
public:
    MainLayer();

    void OnLateUpdate(float deltaTime) override;
    void OnAttach() override;
    void OnUpdate(float deltaTime) override;
    void OnPreFixedUpdate(float deltaTime) override;
    void OnFixedUpdate(float deltaTime) override;
    void OnDetach() override;
    Player* GetPlayer() const { return _player.get(); };

private:
    bool OnGamePaused(Frost::PauseEvent& e);
    bool OnGameUnpaused(Frost::UnPauseEvent& e);

    GameState _gamestate;

    EventHandlerId _pauseHandlerUUID;
    EventHandlerId _unpauseHandlerUUID;

    std::unique_ptr<Terrain> _terrain;
    std::unique_ptr<Water> _water;
    std::unique_ptr<Water> _sndWater;
    std::unique_ptr<Sun> _sun;
    std::unique_ptr<Player> _player;
    std::shared_ptr<Portal> _portal1;
    std::shared_ptr<Portal> _portal2;
    std::shared_ptr<CheckPoint> _checkPoint1;
    std::shared_ptr<CheckPoint> _checkPoint2;
    std::shared_ptr<CheckPoint> _checkPoint3;
    std::shared_ptr<CheckPoint> _checkPoint4;
    std::shared_ptr<CheckPoint> _checkPoint5;
    std::shared_ptr<Wall> _wall;
    std::unique_ptr<Grass> _grass;
};
