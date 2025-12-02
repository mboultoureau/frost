#pragma once
#include "Frost/Scene/Components/HUDText.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/Components/Script.h"
#include "../Player/PlayerScript.h"

namespace Frost
{
    struct Speedometer
    {
        std::shared_ptr<Component::HUDText> hudText;
    };

    class SpeedometerScript : public Script
    {
    public:
        SpeedometerScript(Player& player) : _player(player) {}

        void OnInitialize() override;
        void OnUpdate(float deltaTime) override;

    private:
        Player& _player;

        GameObject _speedometerComponent;

        const float METERS_PER_SECOND_TO_KMH = 3.6f;
    };
} // namespace Frost