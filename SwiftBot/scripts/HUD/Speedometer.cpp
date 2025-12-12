#include "Speedometer.h"
#include "GameState/GameState.h"
#include "Frost/Physics/Physics.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <variant>

#undef min

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void Speedometer::OnCreate()
    {
        auto scene = GetGameObject().GetScene();

        // 1. Find Player Controller

        auto playerObj = scene->FindGameObjectByName("Player");
        if (playerObj.IsValid())
            _playerController = playerObj.GetChildByName("PlayerController");

        // 2. Create UI GameObjects
        _speedometerFrame = scene->CreateGameObject("Speedometer Frame");
        _speedometerNeedle = scene->CreateGameObject("Speedometer Needle");
        _speedometerText = scene->CreateGameObject("Speedometer Text");

        // 3. Setup Frame
        Viewport viewportFrame;
        viewportFrame.x = 0.7f;
        viewportFrame.y = 0.6f;
        viewportFrame.width = 0.3f;
        viewportFrame.height = 0.45f;

        TextureConfig frameConfig = { .textureType = TextureType::HUD, .path = counterPath };
        auto frameTexture = Texture::Create(frameConfig);

        auto& frameElement = _speedometerFrame.AddComponent<UIElement>(
            UIElement{ .content = UIImage{ .texture = frameTexture, .filter = Material::FilterMode::LINEAR } });
        frameElement.viewport = viewportFrame;
        frameElement.priority = 1;

        // 4. Setup Needle
        Viewport viewportNeedle;
        viewportNeedle.x = 0.75f;
        viewportNeedle.y = 0.685f;
        viewportNeedle.width = 0.2f;
        viewportNeedle.height = 0.3f;

        TextureConfig needleConfig = { .textureType = TextureType::HUD, .path = needlePath };
        auto needleTexture = Texture::Create(needleConfig);

        auto& needleElement = _speedometerNeedle.AddComponent<UIElement>(
            UIElement{ .content = UIImage{ .texture = needleTexture, .filter = Material::FilterMode::LINEAR } });
        needleElement.viewport = viewportNeedle;
        needleElement.priority = 2;

        // 5. Setup Text
        auto font = AssetManager::LoadAsset<Font>(fontPath);
        std::string initText = "0.0 km/h";

        auto& textElement = _speedometerText.AddComponent<UIElement>(UIElement{
            .content = UIText{ .text = initText, .font = font, .fontFilepath = fontPath, .fontSize = 1.0f } });

        Viewport viewportText;
        viewportText.x = 0.82f;
        viewportText.y = 0.85f;
        viewportText.width = 0.5f;
        viewportText.height = 0.5f;

        textElement.viewport = viewportText;
        textElement.priority = 3;
        // Couleur définie dans UIElement directement, pas dans UIText
        textElement.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    void Speedometer::OnDestroy()
    {
        auto scene = GetGameObject().GetScene();
        if (scene)
        {
            scene->DestroyGameObject(_speedometerFrame);
            scene->DestroyGameObject(_speedometerNeedle);
            scene->DestroyGameObject(_speedometerText);
        }
    }

    void Speedometer::OnUpdate(float deltaTime)
    {
        if (!_playerController.IsValid() || !_playerController.HasComponent<RigidBody>())
            return;

        // Calculate Speed using Jolt Interface
        auto bodyID = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        float speedMps = Physics::Get().body_interface->GetLinearVelocity(bodyID).Length();
        float speedKmh = speedMps * METERS_PER_SECOND_TO_KMH;

        UpdateSpeedText(speedKmh);
        UpdateNeedle(speedKmh);
    }

    void Speedometer::UpdateSpeedText(float speed)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << speed << " km/h";

        auto& uiElement = _speedometerText.GetComponent<UIElement>();

        if (std::holds_alternative<UIText>(uiElement.content))
        {
            std::get<UIText>(uiElement.content).text = ss.str();
        }
    }

    void Speedometer::UpdateNeedle(float speed)
    {
        float ratio = std::clamp(speed, 0.0f, MAX_SPEED_KMH) / MAX_SPEED_KMH;

        // Calcul: Angle de départ + (Ratio * Amplitude totale)
        // Départ (PI) -> Bas
        // Fin (PI + 270deg) -> Droite (en passant par la gauche et le haut)
        float currentAngle = NEEDLE_START_ANGLE + (ratio * NEEDLE_TOTAL_SWEEP);

        auto& uiElement = _speedometerNeedle.GetComponent<UIElement>();
        uiElement.rotation = currentAngle;
    }
} // namespace GameLogic