#include "Speedometer.h"
#include "../Player/Player.h"
#include "../Player/Vehicles/Vehicle.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Physics/Physics.h"
#include "Game.h"
#include <algorithm>
#include <sstream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void
SpeedometerScript::OnInitialize()
{
    Scene& _scene = Game::GetScene();

    auto font = AssetManager::LoadAsset<Font>("resources/fonts/ss-engine.regular.ttf");

    _speedometerComponent = _scene.CreateGameObject("Speedometer");
    Viewport viewport;
    viewport.x = 0.82f;
    viewport.y = 0.85f;
    viewport.width = 0.5f;
    viewport.height = 0.5f;
    std::string initText = "Speed";
    _speedometerComponent.AddComponent<Frost::Component::HUDText>(viewport, font, initText);

    const std::string SpeedCounter = "resources/textures/counter.png";
    Viewport viewportSpeedCounter;
    viewportSpeedCounter.x = 0.7f;
    viewportSpeedCounter.y = 0.6f;
    viewportSpeedCounter.width = 0.3f;
    viewportSpeedCounter.height = 0.45f;
    _speedometerComponent.AddComponent<Frost::Component::HUDImage>(
        viewportSpeedCounter, SpeedCounter, Frost::Material::FilterMode::LINEAR);

    Viewport viewportSpeedNeedle;
    viewportSpeedNeedle.x = 0.75f;
    viewportSpeedNeedle.y = 0.685f;
    viewportSpeedNeedle.width = 0.2f;
    viewportSpeedNeedle.height = 0.3f;
    const std::string SpeedNeedle = "resources/textures/needle.png";
    _speedometerNeedleComponent = _scene.CreateGameObject("SpeedometerNeedle");
    _speedometerNeedleComponent.AddComponent<Frost::Component::HUDImage>(
        viewportSpeedNeedle, SpeedNeedle, Frost::Material::FilterMode::LINEAR);
}

void
SpeedometerScript::OnUpdate(float deltaTime)
{
    // Speed Update
    float speed = std::abs(Physics::GetBodyInterface().GetLinearVelocity(_playerController.GetBodyID()).Length() *
                           METERS_PER_SECOND_TO_KMH);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << speed << " km/h";
    std::string speedText = ss.str();

    _speedometerComponent.GetComponent<Frost::Component::HUDText>().text = speedText;
    // Needle Rotation Update
    const float MAX_SPEED_KMH = 200.0f;
    const float PI = 3.1415926535f;
    const float RANGE_START = 0.0f;
    const float RANGE_END = 3.0f * PI / 2.0f;
    const float OFFSET = -PI / 4.0f;
    float normalizedSpeed = std::min<float>(speed, MAX_SPEED_KMH) / MAX_SPEED_KMH;
    float angleInScale = RANGE_START + (normalizedSpeed * (RANGE_END - RANGE_START));
    float rotationAngle = angleInScale + OFFSET;

    _speedometerNeedleComponent.GetComponent<Frost::Component::HUDImage>().rotation = rotationAngle;
}