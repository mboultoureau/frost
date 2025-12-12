#pragma once

#include <Frost.h>

namespace GameLogic
{
    class Speedometer : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnDestroy() override;
        void OnUpdate(float deltaTime) override;

    private:
        void UpdateSpeedText(float speed);
        void UpdateNeedle(float speed);

    private:
        // Asset Paths
        const std::string counterPath = "assets/HUD/counter.png";
        const std::string needlePath = "assets/HUD/needle.png";
        const std::string fontPath = "assets/HUD/ss-engine.regular.ttf";

        // GameObjects
        Frost::GameObject _playerController;
        Frost::GameObject _speedometerFrame;
        Frost::GameObject _speedometerNeedle;
        Frost::GameObject _speedometerText;

        // Settings
        const float METERS_PER_SECOND_TO_KMH = 3.6f;
        const float MAX_SPEED_KMH = 200.0f;

        // Needle Math constants
        // Note: Using float literals for PI to avoid header dependency issues if std::numbers not available
        const float PI = 3.1415926535f;
        const float NEEDLE_START_ANGLE = -0.8f; // 180 degrés (Pointe vers le bas)
        const float NEEDLE_TOTAL_SWEEP = 4.8f;  // 270 degrés d'amplitude
    };
} // namespace GameLogic