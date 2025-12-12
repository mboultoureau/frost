#pragma once
#include <Frost.h>
namespace GameLogic
{
    class Timer : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnDestroy() override;
        void OnUpdate(float deltaTime) override;

    private:
        void UpdateCountdown(float deltaTime);
        void UpdateRaceTimer(float deltaTime);
        void StartRace();
        void SetText(const std::string& text);

        Frost::GameObject _timerTextObj;

        const std::string fontPath = "assets/HUD/ss-engine.regular.ttf";

        float _waitTime = 3.0f;
        float _countdownTime = 3.0f;
        float _raceTime = 0.0f;

        bool _isWaiting = true;
        bool _isRaceStarted = false;
        bool _isFinished = false;
    };
} // namespace GameLogic