#pragma once

#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Objects/FreeCamera.h"
#include "Objects/HUD_Logo.h"
#include "Objects/HierarchyTest.h"
#include "Objects/Moto.h"
#include "Objects/Ogre.h"
#include "Objects/OgreNormal.h"
#include "Objects/Plane.h"
#include "Objects/Player.h"
#include "Objects/PointLight.h"
#include "Objects/Portal.h"
#include "Objects/Shapes.h"
#include "Objects/Sky.h"
#include "Objects/Sphere.h"
#include "Objects/SphereCustomShader.h"
#include "Objects/TV.h"
#include "Objects/Text.h"
#include "Objects/TopCamera.h"
#include "Objects/Waves.h"
#include "Objects/Grass.h"
#include <memory>
#include "Objects/Sun.h"

class MainLayer : public Frost::Layer
{
public:
    MainLayer();

    void OnLateUpdate(float deltaTime) override;
    void OnUpdate(float deltaTime) override;
    void OnPreFixedUpdate(float deltaTime) override;
    void OnFixedUpdate(float deltaTime) override;
    void OnAttach() override;
    void OnDetach() override;
    Player* GetPlayer() const { return _player.get(); };

private:
    std::unique_ptr<PointLight> _pointLight;
    std::unique_ptr<Sphere> _sphere;
    std::unique_ptr<FreeCamera> _freeCamera;
    std::unique_ptr<TopCamera> _topCamera;
    std::unique_ptr<Moto> _moto;
    std::unique_ptr<Plane> _plane;
    std::unique_ptr<Ogre> _ogre;
    std::unique_ptr<OgreNormal> _ogreNormal;
    std::unique_ptr<Text> _text;
    std::unique_ptr<Sun> _sun;
    std::unique_ptr<Player> _player;
    std::unique_ptr<HUD_Logo> _hudLogo;
    std::unique_ptr<HierarchyTest> _hierarchy;
    std::unique_ptr<Sky> _sky;
    std::unique_ptr<Portal> _portal1;
    std::unique_ptr<Portal> _portal2;
    std::unique_ptr<TV> _tv;
    std::unique_ptr<Shapes> _shapes;
    std::unique_ptr<Waves> _waves;
    std::unique_ptr<Grass> _grass;
    std::unique_ptr<SphereCustomShader> _sphereCustomShader;

    bool OnGamePaused(Frost::PauseEvent& e);
    bool OnGameUnpaused(Frost::UnPauseEvent& e);

    EventHandlerId _pauseHandlerUUID;
    EventHandlerId _unpauseHandlerUUID;
};
