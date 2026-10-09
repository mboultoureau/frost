#include "Frost/Debugging/DebugInterface/DebugPerformance.h"
#include "Frost/Core/Application.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Asset/Model.h"
#include "Frost/Debugging/Logger.h"

#include <imgui.h>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace Frost
{
    DebugPerformance::DebugPerformance(bool autoBenchmark) : _autoBenchmarkCountdown(autoBenchmark ? 200 : -1) {}

    void DebugPerformance::AddScene(Scene* scene)
    {
        if (std::find(_scenes.begin(), _scenes.end(), scene) == _scenes.end())
        {
            _scenes.push_back(scene);
        }
    }

    void DebugPerformance::RemoveScene(Scene* scene)
    {
        _scenes.erase(std::remove(_scenes.begin(), _scenes.end(), scene), _scenes.end());
    }

    void DebugPerformance::ClearScenes()
    {
        _scenes.clear();
    }

    BenchmarkStats DebugPerformance::RunBenchmark()
    {
        FT_ENGINE_INFO("=== Starting Performance Benchmark ===");

        // Scene metrics
        size_t totalEntityCount = 0;
        size_t totalPolygonCount = 0;
        size_t totalVertexCount = 0;
        for (Scene* scene : _scenes)
        {
            if (scene)
            {
                totalEntityCount += scene->GetEntityCount();
                totalPolygonCount += scene->GetPolygonCount();
                totalVertexCount += scene->GetVertexCount();
            }
        }
        _benchmarkStats.sceneEntityCount = totalEntityCount;
        _benchmarkStats.scenePolygonCount = totalPolygonCount;
        _benchmarkStats.sceneVertexCount = totalVertexCount;

        // Frame time metrics
        float minFt = 999999.0f;
        float maxFt = 0.0f;
        float sumFt = 0.0f;
        int validCount = 0;
        for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
        {
            if (_frameTimes[i] > 0.0f)
            {
                if (_frameTimes[i] < minFt)
                    minFt = _frameTimes[i];
                if (_frameTimes[i] > maxFt)
                    maxFt = _frameTimes[i];
                sumFt += _frameTimes[i];
                validCount++;
            }
        }
        _benchmarkStats.avgFrameTimeMs = (validCount > 0) ? (sumFt / validCount) : 16.6f;
        _benchmarkStats.minFrameTimeMs = (minFt < 999999.0f) ? minFt : _benchmarkStats.avgFrameTimeMs;
        _benchmarkStats.maxFrameTimeMs = maxFt;
        _benchmarkStats.avgFps =
            (_benchmarkStats.avgFrameTimeMs > 0.0f) ? (1000.0f / _benchmarkStats.avgFrameTimeMs) : 0.0f;

        float sumPhys = 0.0f;
        int validPhys = 0;
        for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
        {
            if (_fixedUpdateTimes[i] > 0.0f)
            {
                sumPhys += _fixedUpdateTimes[i];
                validPhys++;
            }
        }
        _benchmarkStats.avgPhysicsTimeMs = (validPhys > 0) ? (sumPhys / validPhys) : 0.0f;

        // Asset Loading Benchmark (Mono vs Multi-Thread)
        std::vector<std::string> validModels;
        std::filesystem::path searchRoot = Application::GetProjectDirectory();
        if (searchRoot.empty())
        {
            searchRoot = std::filesystem::current_path();
        }

        std::filesystem::path assetsDir = searchRoot / "assets";
        if (std::filesystem::exists(assetsDir))
        {
            searchRoot = assetsDir;
        }

        try
        {
            if (std::filesystem::exists(searchRoot))
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(
                         searchRoot, std::filesystem::directory_options::skip_permission_denied))
                {
                    if (entry.is_regular_file())
                    {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        if (ext == ".glb" || ext == ".gltf" || ext == ".obj" || ext == ".fbx")
                        {
                            validModels.push_back(entry.path().string());
                            if (validModels.size() >= 12)
                                break;
                        }
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            FT_ENGINE_WARN("Could not scan for benchmark models: {}", e.what());
        }

        if (!validModels.empty())
        {
            // Single-threaded loading
            auto monoStart = std::chrono::high_resolution_clock::now();
            for (const auto& path : validModels)
            {
                Model model;
                model.LoadCPU(path);
            }
            auto monoEnd = std::chrono::high_resolution_clock::now();
            _benchmarkStats.assetLoadMonoMs = std::chrono::duration<float, std::milli>(monoEnd - monoStart).count();

            // Multi-threaded loading
            auto multiStart = std::chrono::high_resolution_clock::now();
            std::vector<std::thread> workers;
            workers.reserve(validModels.size());
            for (const auto& path : validModels)
            {
                workers.emplace_back(
                    [path]()
                    {
                        Model model;
                        model.LoadCPU(path);
                    });
            }
            for (auto& w : workers)
            {
                if (w.joinable())
                    w.join();
            }
            auto multiEnd = std::chrono::high_resolution_clock::now();
            _benchmarkStats.assetLoadMultiMs = std::chrono::duration<float, std::milli>(multiEnd - multiStart).count();

            _benchmarkStats.assetsTestedCount = validModels.size();
            _benchmarkStats.assetSpeedup = (_benchmarkStats.assetLoadMultiMs > 0.0f)
                                               ? (_benchmarkStats.assetLoadMonoMs / _benchmarkStats.assetLoadMultiMs)
                                               : 1.0f;
        }

        // ECS Stress Test (10 000 entities)
        {
            constexpr size_t TEST_ENTITY_COUNT = 10000;
            Scene stressScene("StressTestBenchmark");
            auto ecsStart = std::chrono::high_resolution_clock::now();
            for (size_t i = 0; i < TEST_ENTITY_COUNT; ++i)
            {
                stressScene.CreateGameObject("Entity_" + std::to_string(i));
            }
            auto ecsEnd = std::chrono::high_resolution_clock::now();
            _benchmarkStats.ecsCreationMs = std::chrono::duration<float, std::milli>(ecsEnd - ecsStart).count();

            auto updateStart = std::chrono::high_resolution_clock::now();
            stressScene.PreFixedUpdate(0.016f);
            auto updateEnd = std::chrono::high_resolution_clock::now();
            _benchmarkStats.ecsUpdateMs = std::chrono::duration<float, std::milli>(updateEnd - updateStart).count();
            _benchmarkStats.ecsEntitiesTested = TEST_ENTITY_COUNT;
        }

        _benchmarkStats.completed = true;

        ExportStatsToFile();

        FT_ENGINE_INFO("=== Benchmark Completed ===");
        FT_ENGINE_INFO("Scene Entities: {}, Polygons: {}, Vertices: {}",
                       _benchmarkStats.sceneEntityCount,
                       _benchmarkStats.scenePolygonCount,
                       _benchmarkStats.sceneVertexCount);
        FT_ENGINE_INFO("Frame Time Avg: {:.2f} ms ({:.1f} FPS), Min: {:.2f} ms, Max: {:.2f} ms",
                       _benchmarkStats.avgFrameTimeMs,
                       _benchmarkStats.avgFps,
                       _benchmarkStats.minFrameTimeMs,
                       _benchmarkStats.maxFrameTimeMs);
        FT_ENGINE_INFO("Physics Update Avg: {:.2f} ms", _benchmarkStats.avgPhysicsTimeMs);
        FT_ENGINE_INFO("Asset Loading: Mono = {:.2f} ms, Multi = {:.2f} ms, Speedup = {:.2f}x ({:.1f}% faster)",
                       _benchmarkStats.assetLoadMonoMs,
                       _benchmarkStats.assetLoadMultiMs,
                       _benchmarkStats.assetSpeedup,
                       (_benchmarkStats.assetLoadMonoMs > 0.0f
                            ? (1.0f - _benchmarkStats.assetLoadMultiMs / _benchmarkStats.assetLoadMonoMs) * 100.0f
                            : 0.0f));
        FT_ENGINE_INFO("ECS ({} entities): Creation = {:.2f} ms, Transform Update = {:.2f} ms",
                       _benchmarkStats.ecsEntitiesTested,
                       _benchmarkStats.ecsCreationMs,
                       _benchmarkStats.ecsUpdateMs);

        return _benchmarkStats;
    }

    void DebugPerformance::ExportStatsToFile(const std::string& filepath)
    {
        std::filesystem::path targetPath = filepath;
        if (!Application::GetProjectDirectory().empty())
        {
            targetPath = Application::GetProjectDirectory() / filepath;
        }

        std::ofstream out(targetPath);
        if (!out.is_open())
        {
            out.open(filepath);
            if (!out.is_open())
                return;
        }

        out << "========================================================\n";
        out << "        FROST ENGINE - PERFORMANCE BENCHMARK REPORT      \n";
        out << "========================================================\n\n";

#ifdef FT_RELEASE
        out << "Build Configuration:    Release (Optimized /O2)\n\n";
#else
        out << "Build Configuration:    Debug\n\n";
#endif

        out << "--- FRAME RENDERING & TIMING ---\n";
        out << "Average Frame Time:     " << _benchmarkStats.avgFrameTimeMs << " ms\n";
        out << "Average Frame Rate:     " << _benchmarkStats.avgFps << " FPS\n";
        out << "Min Frame Time:         " << _benchmarkStats.minFrameTimeMs << " ms\n";
        out << "Max Frame Time:         " << _benchmarkStats.maxFrameTimeMs << " ms\n";
        out << "Physics Update Time:    " << _benchmarkStats.avgPhysicsTimeMs << " ms\n\n";

        out << "--- SCENE CAPACITY & GEOMETRY ---\n";
        out << "Active Entities:        " << _benchmarkStats.sceneEntityCount << "\n";
        out << "Total Polygons (Tris):  " << _benchmarkStats.scenePolygonCount << "\n";
        out << "Total Vertices:         " << _benchmarkStats.sceneVertexCount << "\n\n";

        out << "--- MULTI-THREADING vs SINGLE-THREAD (ASSET LOADING) ---\n";
        out << "Assets Tested:          " << _benchmarkStats.assetsTestedCount << " 3D models (Assimp/GLTF/FBX)\n";
        out << "Single-Thread Time:     " << _benchmarkStats.assetLoadMonoMs << " ms\n";
        out << "Multi-Thread Time:      " << _benchmarkStats.assetLoadMultiMs << " ms\n";
        float reduction = (_benchmarkStats.assetLoadMonoMs > 0.0f)
                              ? (1.0f - _benchmarkStats.assetLoadMultiMs / _benchmarkStats.assetLoadMonoMs) * 100.0f
                              : 0.0f;
        out << "Speedup Factor:         " << _benchmarkStats.assetSpeedup << "x (" << reduction
            << "% time reduction)\n\n";

        out << "--- ECS SCALABILITY & STRESS TEST ---\n";
        out << "ECS Entities Tested:    " << _benchmarkStats.ecsEntitiesTested << " entities (EnTT)\n";
        out << "Creation Time:          " << _benchmarkStats.ecsCreationMs << " ms\n";
        out << "Transform Update Time:  " << _benchmarkStats.ecsUpdateMs << " ms\n";
        if (_benchmarkStats.ecsUpdateMs > 0.0f)
        {
            out << "Update Throughput:      "
                << (_benchmarkStats.ecsEntitiesTested / (_benchmarkStats.ecsUpdateMs / 1000.0f)) << " entities/sec\n";
        }
        out << "\n========================================================\n";
    }

    void DebugPerformance::OnImGuiRender(float deltaTime)
    {
        if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float currentFrameTime =
                _frameTimes[(_frameTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) % FRAME_TIME_HISTORY_SIZE];
            float fps = (currentFrameTime > 0.0f) ? (1000.0f / currentFrameTime) : 0.0f;

            ImGui::Text("Frame Time (Total): %.2f ms (FPS: %.0f)", currentFrameTime, fps);

            size_t totalEntityCount = 0;
            size_t totalPolygonCount = 0;
            size_t totalVertexCount = 0;
            for (Scene* scene : _scenes)
            {
                if (scene)
                {
                    totalEntityCount += scene->GetEntityCount();
                    totalPolygonCount += scene->GetPolygonCount();
                    totalVertexCount += scene->GetVertexCount();
                }
            }
            ImGui::Text("Total Entities: %zu", totalEntityCount);
            ImGui::Text("Total Polygons (Tris): %zu", totalPolygonCount);
            ImGui::Text("Total Vertices: %zu", totalVertexCount);

            float averageFrameTime = 0.0f;
            float minFt = 999999.0f;
            for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
            {
                if (_frameTimes[i] > 0.0f && _frameTimes[i] < minFt)
                    minFt = _frameTimes[i];
                averageFrameTime += _frameTimes[i];
            }
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

            ImGui::Separator();

            bool useMulti = AssetManager::GetUseMultiThreading();
            if (ImGui::Checkbox("AssetManager Multi-Threading", &useMulti))
            {
                AssetManager::SetUseMultiThreading(useMulti);
            }
            ImGui::Text("Active Loading Threads: %d", AssetManager::GetActiveLoadingThreads());

            ImGui::Separator();
            if (ImGui::Button("Run Benchmark Suite (Mono vs Multi)"))
            {
                RunBenchmark();
            }

            if (_benchmarkStats.completed)
            {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "Benchmark Results:");
                ImGui::Text("  Assets Tested: %zu models", _benchmarkStats.assetsTestedCount);
                ImGui::Text("  Single-Thread Loading: %.2f ms", _benchmarkStats.assetLoadMonoMs);
                ImGui::Text("  Multi-Thread Loading:  %.2f ms", _benchmarkStats.assetLoadMultiMs);
                ImGui::Text("  Speedup: %.2fx", _benchmarkStats.assetSpeedup);
                ImGui::Text("  ECS Stress (%zu ents): Create %.2f ms, Update %.2f ms",
                            _benchmarkStats.ecsEntitiesTested,
                            _benchmarkStats.ecsCreationMs,
                            _benchmarkStats.ecsUpdateMs);
            }
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

        if (_autoBenchmarkCountdown > 0)
        {
            _autoBenchmarkCountdown--;
            if (_autoBenchmarkCountdown == 100)
            {
                for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
                {
                    _frameTimes[i] = 0.0f;
                    _fixedUpdateTimes[i] = 0.0f;
                }
                _frameTimeHistoryIndex = 0;
                _fixedUpdateTimeHistoryIndex = 0;
            }
            else if (_autoBenchmarkCountdown == 0)
            {
                RunBenchmark();
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