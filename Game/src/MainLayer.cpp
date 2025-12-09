#include "MainLayer.h"
#include "Game.h"
#include "Objects/Boost.h"
#include "Objects/RenderingObjects/Sky.h"
#include "Objects/LapCheckPoint.h"
#include "Objects/LevelCamera.h"
#include "Objects/Player/Vehicles/Vehicle.h"
#include "Objects/Terrain.h"
#include "Objects/Wall.h"
#include "Objects/Water.h"
using namespace Frost;

MainLayer::MainLayer() : Layer("MainLayer") {}

void
MainLayer::OnAttach()
{
    Scene& _scene = Game::GetScene();
    _terrain = std::make_unique<Terrain>(Terrain());
    //_wall = std::make_unique<Wall>(Wall());

    _gamestate = GameState();

    _gamestate.SetLap(2);

    // player

    // portal

    //_terrain = std::make_unique<Terrain>();
    _water = std::make_unique<Water>(Vector3(-10.0, -70.0, 50.0), EulerAngles(), Vector3(50, 230, 50), 0.2f);
    _sndWater = std::make_unique<Water>(Vector3(-50, 65, -30.0), EulerAngles(), Vector3(15, 10, 20), 0.2f);
    //_sndWater = std::make_unique<Water>(Vector3(-315, 69, -32), EulerAngles(), Vector3(20, 15, 20), 0.2f);

    _sun = std::make_unique<Sun>();

    _checkPoint1 = std::make_shared<LapCheckPoint>(Vector3{ -14, 52, 78 }, _gamestate);
    _checkPoint2 = std::make_shared<CheckPoint>(Vector3{ -14, 61, 17 });
    _checkPoint3 = std::make_shared<CheckPoint>(Vector3{ -17, 61, -111 });

    auto arrow = Game::GetScene().CreateGameObject("Arrow Billboard");
    arrow.AddComponent<Billboard>(Vector3(-17, 50, -50), 5.0f, "resources/textures/arrow-up.png");

    _grass = std::make_unique<Grass>(Vector3{ 9.0f, 60.7f, -47 }, EulerAngles(180_deg, 0_deg, 0_deg), Vector3(1, 1, 1));

    _boost =
        std::make_shared<Boost>(Vector3{ -13.0f, 61.7f, -10 }, EulerAngles(), Vector3(10, 2, 2), Vector3(0, 1, 0), 2);

    // link 1 / 2
    _checkPoint1->AddChild(_checkPoint2);
    _checkPoint2->AddParent(_checkPoint1);

    // link 2 / 3
    _checkPoint2->AddChild(_checkPoint3);
    _checkPoint3->AddParent(_checkPoint2);

    // link 1 / 3
    _checkPoint3->AddChild(_checkPoint1);
    _checkPoint1->AddParent(_checkPoint3);

    _checkPoint1->ActivatePhysics();

    // LevelCamera levelCamera;
    auto _sky = std::make_unique<Sky>();

    _startMenu = std::make_unique<Frost::StartMenu>();
    _startMenu->OnAttach();

    _pauseHandlerUUID = EventManager::Subscribe<Frost::PauseEvent>(FROST_BIND_EVENT_FN(MainLayer::OnGamePaused));

    _unpauseHandlerUUID = EventManager::Subscribe<Frost::UnPauseEvent>(FROST_BIND_EVENT_FN(MainLayer::OnGameUnpaused));
}

void
MainLayer::OnUpdate(float deltaTime)
{
    Frost::Scene& _scene = Game::GetScene();

    _scene.Update(deltaTime);
}

void
MainLayer::OnLateUpdate(float deltaTime)
{
    Frost::Scene& _scene = Game::GetScene();
    _scene.LateUpdate(deltaTime);
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

    static bool winScreenCreated = false;

    if (!_gamestate.IsGameStarted())
    {
        _startMenu->OnUpdate(deltaTime);
        if (!_paused)
        {
            _gamestate.StartGame();
            _player = std::make_unique<Player>();
            _portal1 = std::make_shared<Portal>(Vector3{ -18, 60, -110 },
                                                EulerAngles{ 0_deg, 180_deg, 0_deg },
                                                Vector3{ 3.0f, 3.0f, 3.0f },
                                                _player.get());
            _portal2 = std::make_shared<Portal>(Vector3{ -22, 51, 87 },
                                                EulerAngles{ 0_deg, 135_deg, 0_deg },
                                                Vector3{ 3.0f, 3.0f, 3.0f },
                                                _player.get());

            _portal1->SetupPortal(PortalType::Entry, _portal2->_portal);
            _portal2->SetupPortal(PortalType::Exit);

            _portal3 = std::make_shared<Portal>(Vector3{ -23, 45, 29 },
                                                EulerAngles{ 0_deg, 180_deg, 0_deg },
                                                Vector3{ 3.0f, 3.0f, 3.0f },
                                                _player.get());
            _portal4 = std::make_shared<Portal>(Vector3{ -23, 60, 23 },
                                                EulerAngles{ 0_deg, 180_deg, 0_deg },
                                                Vector3{ 3.0f, 3.0f, 3.0f },
                                                _player.get());

            _portal3->SetupPortal(PortalType::Entry, _portal4->_portal);
            _portal4->SetupPortal(PortalType::Exit);

            _startMenu->OnDetach();
            _startMenu.reset();
        }
    }
    if (_gamestate.Finished())
    {

        if (!winScreenCreated)
        {
            EventManager::Emit<Frost::PauseEvent>();

            auto winScreen = _scene.CreateGameObject("victoryScreen");

            Viewport viewportImage;
            viewportImage.height = 0.7f;
            viewportImage.width = 0.6f;
            viewportImage.x = 0.2f;
            viewportImage.y = 0.1f;
            const std::string WIN_PATH = "resources/textures/victoryScreen.png";
            Viewport viewportButton;
            viewportButton.height = 0.2f;
            viewportButton.width = 0.2f;
            viewportButton.x = 0.4f;
            viewportButton.y = 0.8f;
            const std::string hover = "resources/textures/reload-hover.png";
            const std::string idle = "resources/textures/reload-idle.png";
            const std::string press = "resources/textures/reload-press.png";

            Scene& _scene = Game::GetScene();

            auto VictoryScreen = _scene.CreateGameObject("Victory Screen");
            _scene.AddComponent<HUDImage>(winScreen, viewportImage, WIN_PATH, Material::FilterMode::POINT);

            VictoryScreen.AddComponent<UIButton>(viewportButton, hover, idle, press, [this]() { OnPressedButton(); });
            auto& victoryButton = VictoryScreen.GetComponent<Frost::Component::UIButton>();
            victoryButton.buttonHitbox = viewportButton;
            winScreenCreated = true;
        }
    }
    else
    {

        if (winScreenCreated)
        {
            winScreenCreated = false;
        }

        _checkPoint1->FixedUpdate(deltaTime);
        _checkPoint2->FixedUpdate(deltaTime);
        _checkPoint3->FixedUpdate(deltaTime);
    }
    _scene.FixedUpdate(deltaTime);
}
void
MainLayer::OnPressedButton()
{
    EventManager::Emit<Frost::ResetEvent>();
}

void
MainLayer::OnDetach()
{
    Player::ResetPlayers();
    EventManager::Unsubscribe<PauseEvent>(_pauseHandlerUUID);
    EventManager::Unsubscribe<UnPauseEvent>(_unpauseHandlerUUID);
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