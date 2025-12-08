#pragma once
#pragma once
#include "Frost/Scene/Components/HUDText.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/Components/Script.h"
#include "../Player/PlayerScript.h"

namespace Frost
{
    struct RaceTimer
    {
        std::shared_ptr<Component::HUDText> hudText;
    };

    class RaceTimerScript : public Script
    {
    public:
        RaceTimerScript(Player& player) : _player(player) {}

        void OnInitialize() override;
        void UpdateTimerInRace();
        void OnUpdate(float deltaTime) override;

    private:
        Player& _player;

        GameObject _raceTimerCompenent;
        Timer _internalTimer;
        bool _raceStarted = false;
    };
} // namespace Frost