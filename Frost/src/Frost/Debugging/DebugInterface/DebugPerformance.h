#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Debugging/DebugInterface/DebugPanel.h"
#include "Frost/Scene/Scene.h"
#include <vector>
#include <string>

namespace Frost
{
    struct BenchmarkStats
    {
        bool completed = false;
        float assetLoadMonoMs = 0.0f;
        float assetLoadMultiMs = 0.0f;
        float assetSpeedup = 1.0f;
        size_t assetsTestedCount = 0;

        size_t ecsEntitiesTested = 0;
        float ecsCreationMs = 0.0f;
        float ecsUpdateMs = 0.0f;

        size_t sceneEntityCount = 0;
        size_t scenePolygonCount = 0;
        size_t sceneVertexCount = 0;

        float avgFrameTimeMs = 0.0f;
        float minFrameTimeMs = 0.0f;
        float maxFrameTimeMs = 0.0f;
        float avgFps = 0.0f;
        float avgPhysicsTimeMs = 0.0f;
    };

    class FROST_API DebugPerformance : public DebugPanel
    {
    public:
        DebugPerformance(bool autoBenchmark = true);
        virtual ~DebugPerformance() override = default;
        virtual void OnImGuiRender(float deltaTime) override;
        virtual void OnLateUpdate(float deltaTime) override;
        virtual void OnFixedUpdate(float fixedDeltaTime) override;
        virtual const char* GetName() const override { return "Performance"; }

        void AddScene(Scene* scene);
        void RemoveScene(Scene* scene);
        void ClearScenes();

        BenchmarkStats RunBenchmark();
        void ExportStatsToFile(const std::string& filepath = "frost_performance_stats.txt");
        const BenchmarkStats& GetBenchmarkStats() const { return _benchmarkStats; }

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

        std::vector<Scene*> _scenes;

        BenchmarkStats _benchmarkStats;
        int _autoBenchmarkCountdown = 200;
    };
} // namespace Frost