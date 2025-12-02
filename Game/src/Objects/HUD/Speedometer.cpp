#include "Speedometer.h"
#include "../Player/Player.h"
#include "../Player/Vehicles/Vehicle.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Physics/Physics.h"
#include "Game.h"

#include <sstream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void
SpeedometerScript::OnInitialize()
{
    Scene& _scene = Game::GetScene();

    auto font =
        AssetManager::LoadAsset<Font>("resources/fonts/ss-engine.regular.ttf", "resources/fonts/ss-engine.regular.ttf");

    _speedometerComponent = _scene.CreateGameObject("Speedometer");
    Viewport viewport;
    viewport.x = 0.82f;
    viewport.y = 0.0f;
    viewport.width = 0.5f;
    viewport.height = 0.5f;
    std::string initText = "Speed";
    _speedometerComponent.AddComponent<Frost::Component::HUDText>(viewport, font, initText);
}

void
SpeedometerScript::OnUpdate(float deltaTime)
{
    float speed = std::abs(Physics::GetBodyInterface().GetLinearVelocity(_player.GetBodyID()).Length() *
                           METERS_PER_SECOND_TO_KMH);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << speed << " km/h";
    std::string speedText = ss.str();

    _speedometerComponent.GetComponent<Frost::Component::HUDText>().text = speedText;
}