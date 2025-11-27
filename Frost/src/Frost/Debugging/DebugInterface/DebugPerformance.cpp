#include "Frost/Debugging/DebugInterface/DebugPerformance.h"

#include <imgui.h>
#include <string>

namespace Frost
{
    void DebugPerformance::OnImGuiRender(float deltaTime)
    {
        if (ImGui::CollapsingHeader("Performance"))
        {
            float currentFrameTime =
                _frameTimes[(_frameTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) % FRAME_TIME_HISTORY_SIZE];
            float fps = (currentFrameTime > 0.0f) ? (1000.0f / currentFrameTime) : 0.0f;

            ImGui::Text("Frame Time (Total): %.2f ms (FPS: %.0f)", currentFrameTime, fps);

            float averageFrameTime = 0.0f;
            for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
                averageFrameTime += _frameTimes[i];
            averageFrameTime /= FRAME_TIME_HISTORY_SIZE;
            std::string overlayFrame = "Avg: " + std::to_string(static_cast<int>(averageFrameTime)) +
                                       "ms | Max: " + std::to_string(static_cast<int>(_maxFrameTime)) + "ms";

            ImGui::PlotLines("Frame Time (ms)",
                             _frameTimes,
                             FRAME_TIME_HISTORY_SIZE,
                             _frameTimeHistoryIndex,
                             overlayFrame.c_str(),
                             0.0f,
                             _maxFrameTime * 1.2f,
                             ImVec2(0, 80.0f));

            ImGui::Separator();

            float currentFixedTime = _fixedUpdateTimes[(_fixedUpdateTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) %
                                                       FRAME_TIME_HISTORY_SIZE];

            ImGui::Text("Physics Update: %.2f ms", currentFixedTime);

            float averageFixedUpdateTime = 0.0f;
            for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
            {
                averageFixedUpdateTime += _fixedUpdateTimes[i];
            }
            averageFixedUpdateTime /= FRAME_TIME_HISTORY_SIZE;

            std::string overlayFixed = "Avg: " + std::to_string(static_cast<int>(averageFixedUpdateTime)) +
                                       "ms | Max: " + std::to_string(static_cast<int>(_maxFixedUpdateTime)) + "ms";

            ImGui::PlotLines("Physics Time (ms)",
                             _fixedUpdateTimes,
                             FRAME_TIME_HISTORY_SIZE,
                             _fixedUpdateTimeHistoryIndex,
                             overlayFixed.c_str(),
                             0.0f,
                             _maxFixedUpdateTime * 1.2f,
                             ImVec2(0, 80.0f));
        }
    }

    void DebugPerformance::OnLateUpdate(float deltaTime)
    {
        float frameTimeMs = deltaTime * 1000.0f;

        // Update history buffer (circular buffer)
        _frameTimes[_frameTimeHistoryIndex] = frameTimeMs;
        _frameTimeHistoryIndex = (_frameTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_SIZE;

        // Update max frame time for graph scaling
        _maxFrameTime = 0.0f;
        for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
        {
            if (_frameTimes[i] > _maxFrameTime)
            {
                _maxFrameTime = _frameTimes[i];
            }
        }
    }

    void DebugPerformance::OnFixedUpdate(float fixedDeltaTime)
    {
        // record time in milliseconds
        float timeMs = fixedDeltaTime * 1000.0f;

        // update history buffer (circular buffer)
        _fixedUpdateTimes[_fixedUpdateTimeHistoryIndex] = timeMs;
        _fixedUpdateTimeHistoryIndex = (_fixedUpdateTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_SIZE;

        // update max time for graph scaling
        if (timeMs > _maxFixedUpdateTime)
        {
            _maxFixedUpdateTime = timeMs;
        }
    }
} // namespace Frost