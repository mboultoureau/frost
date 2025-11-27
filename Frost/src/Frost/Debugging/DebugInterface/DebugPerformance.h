#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"

namespace Frost
{
    class DebugPerformance : public DebugPanel
    {
    public:
        DebugPerformance() = default;
        virtual ~DebugPerformance() override = default;
        virtual void OnImGuiRender(float deltaTime) override;
        virtual void OnLateUpdate(float deltaTime) override;
        virtual void OnFixedUpdate(float fixedDeltaTime) override;
        virtual const char* GetName() const override { return "Performance"; }

    private:
        static constexpr int FRAME_TIME_HISTORY_SIZE = 100;

        // Render update graph
        float _frameTimes[FRAME_TIME_HISTORY_SIZE] = {};
        int _frameTimeHistoryIndex = 0;
        float _maxFrameTime = 0.0f;

        // Physics update graph
        float _fixedUpdateTimes[FRAME_TIME_HISTORY_SIZE] = {};
        int _fixedUpdateTimeHistoryIndex = 0;
        float _maxFixedUpdateTime = 0.0f;
    };
} // namespace Frost