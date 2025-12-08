#include "RaceTimer.h"
#include "../Player/Player.h"
#include "../../MainLayer.h"
#include "../Player/Vehicles/Vehicle.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Physics/Physics.h"
#include "Game.h"
#include <algorithm>
#include <sstream>
#include <Frost/Event/Events/PauseMenu/PauseEvent.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void
RaceTimerScript::OnInitialize()
{
    Scene& _scene = Game::GetScene();

    auto font = AssetManager::LoadAsset<Font>("resources/fonts/ss-engine.regular.ttf");

    _raceTimerCompenent = _scene.CreateGameObject("Speedometer");
    Viewport viewport;
    viewport.x = 0.45f;
    viewport.y = 0.4f;
    viewport.width = 0.5f;
    viewport.height = 0.5f;
    std::string initText = "3";
    _raceTimerCompenent.AddComponent<Frost::Component::HUDText>(viewport, font, initText);
    _raceTimerCompenent.GetComponent<Frost::Component::HUDText>().fontSize = 5;
    _internalTimer.Start();
}

void
RaceTimerScript::UpdateTimerInRace()
{
    auto mainLayer = Game::GetMainLayer();
    if (mainLayer->GetGameState().Finished())
    {
        return;
    }

    if (_raceStarted)
    {
        auto ms = _internalTimer.GetDurationAs<std::chrono::milliseconds>();

        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms - minutes);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(ms - minutes - seconds);

        std::string formatted =
            std::format("{:02}:{:02}:{:03}", minutes.count(), seconds.count(), milliseconds.count());

        _raceTimerCompenent.GetComponent<Frost::Component::HUDText>().text = formatted;
    }
    else
    {
        auto ms = _internalTimer.GetDurationAs<std::chrono::milliseconds>();

        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms - minutes);

        int remaining = 3 - seconds.count();

        std::string cDown = std::to_string(remaining);

        _raceTimerCompenent.GetComponent<Frost::Component::HUDText>().text = cDown;

        if (remaining == 0)
        {
            _raceStarted = true;
            _player.allowInput = true;
            _internalTimer.Start();
            Viewport viewport;
            viewport.x = 0.05f;
            viewport.y = 0.05f;
            viewport.width = 0.5f;
            viewport.height = 0.5f;
            std::string initText = "0";
            auto font = AssetManager::LoadAsset<Font>("resources/fonts/ss-engine.regular.ttf");
            _raceTimerCompenent.AddComponent<Frost::Component::HUDText>(viewport, font, initText);
        }
    }
}

void
RaceTimerScript::OnUpdate(float deltaTime)
{
    UpdateTimerInRace();
}