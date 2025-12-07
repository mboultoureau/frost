#pragma once

#include <string>
#include <vector>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Editor
{
    struct BroadPhaseLayerSetting
    {
        std::string name = "Layer";
        uint8_t layerId = 0;
    };

    struct ObjectLayerSetting
    {
        std::string name = "Layer";
        JPH::ObjectLayer layerId = 0;
        uint8_t broadPhaseLayerId = 0;
    };

    struct ProjectConfig
    {
        // General
        std::string name;
        std::string version;
        std::string startScene;
        std::string assetDirectory;
        std::string sourceDirectory;
        std::string scriptingModule;

        // Physics
        std::vector<BroadPhaseLayerSetting> broadPhaseLayers;
        std::vector<ObjectLayerSetting> objectLayers;

        std::vector<bool> broadPhaseCollisionMatrix;
        std::vector<bool> objectCollisionMatrix;
        std::vector<bool> objectBroadPhaseCollisionMatrix;
    };

    class ProjectInfo
    {
    public:
        ProjectInfo() = default;

        const ProjectConfig& GetConfig() const { return _config; }
        const std::string& GetProjectDir() const { return _projectDir; }
        const std::string& GetProjectFilePath() const { return _projectFilePath; }

        void Clear();
        bool Save();

        bool LoadFromPath(const std::string& path);
        ProjectConfig& GetConfigRef() { return _config; }

        static bool CreateNewProject(const std::string& name,
                                     const std::string& parentDir,
                                     const std::string& templatePath);

    private:
        ProjectConfig _config;
        std::string _projectDir;
        std::string _projectFilePath;

        constexpr static const char* VERSION = "1.0.0";
    };
} // namespace Editor