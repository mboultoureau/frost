#include "MainLayer.h"
#include "Game.h"
#include "Objects/BallRain.h"

using namespace Frost;
using namespace Frost::Math;

MainLayer::MainLayer() : Layer("MainLayer") {}

void
MainLayer::OnAttach()
{
    Frost::Scene& _scene = Game::GetScene();

    _pointLight = std::make_unique<PointLight>();
    //_topCamera = std::make_unique<TopCamera>();
    _sun = std::make_unique<Sun>();
    _sphere = std::make_unique<Sphere>();
    //_freeCamera = std::make_unique<FreeCamera>();
    _moto = std::make_unique<Moto>();
    _ogre = std::make_unique<Ogre>();
    _ogreNormal = std::make_unique<OgreNormal>();
    //_text = std::make_unique<Text>();
    //_plane = std::make_unique<Plane>();
    _player = std::make_unique<Player>();
    //_hierarchy = std::make_unique<HierarchyTest>();
    // auto _rain = std::make_unique<BallRain>();
    //_hudLogo = std::make_unique<HUD_Logo>();
    _sky = std::make_unique<Sky>();
    _portal1 = std::make_unique<Portal>(Vector3{ -15, 51, 90 }, EulerAngles{ 0.0_deg, 220.0_deg, 0.0_deg });
    _portal2 = std::make_unique<Portal>(Vector3{ -6.0f, 0.0f, 0.0f }, EulerAngles{ 0.0_deg, 150.0_deg, 0.0_deg });

    _portal1->LinkTo(_portal2.get());
    _portal2->LinkTo(_portal1.get());

    _grass = std::make_unique<Grass>();
    _portal1->LinkTo(_portal2.get());
    _portal2->LinkTo(_portal1.get());
    _tv = std::make_unique<TV>();
    _sphereCustomShader = std::make_unique<SphereCustomShader>();
    _shapes = std::make_unique<Shapes>();
    _waves = std::make_unique<Waves>();

    _pauseHandlerUUID = EventManager::Subscribe<Frost::PauseEvent>(FROST_BIND_EVENT_FN(MainLayer::OnGamePaused));

    _unpauseHandlerUUID = EventManager::Subscribe<Frost::UnPauseEvent>(FROST_BIND_EVENT_FN(MainLayer::OnGameUnpaused));
}

void
MainLayer::OnUpdate(float deltaTime)
{
    Frost::Scene& _scene = Game::GetScene();

    if (_portal1)
        _portal1->Update();
    if (_portal2)
        _portal2->Update();

    _waves->Update(deltaTime);
    _grass->Update(deltaTime);

    _pauseHandlerUUID = EventManager::Subscribe<Frost::PauseEvent>(FROST_BIND_EVENT_FN(MainLayer::OnGamePaused));

    _unpauseHandlerUUID = EventManager::Subscribe<Frost::UnPauseEvent>(FROST_BIND_EVENT_FN(MainLayer::OnGameUnpaused));
    _scene.Update(deltaTime);
}

void
MainLayer::OnPreFixedUpdate(float deltaTime)
{
    Frost::Scene& _scene = Game::GetScene();

    _scene.PreFixedUpdate(deltaTime);
}

void
MainLayer::OnFixedUpdate(float deltaTime)
{
    Frost::Scene& _scene = Game::GetScene();

    _scene.FixedUpdate(deltaTime);
}

void
MainLayer::OnLateUpdate(float deltaTime)
{
    Frost::Scene& _scene = Game::GetScene();
    _scene.LateUpdate(deltaTime);
}

void
MainLayer::OnDetach()
{
    // EventManager::Unsubscribe<EventType::GamePaused>(_pauseHandlerUUID);
    // EventManager::Unsubscribe<EventType::GameUnpaused>(_unpauseHandlerUUID);
}

bool
MainLayer::OnGamePaused(Frost::PauseEvent& e)
{
    _paused = true;
    return true;
}

bool
MainLayer::OnGameUnpaused(Frost::UnPauseEvent& e)
{
    _paused = false;
    return true;
}