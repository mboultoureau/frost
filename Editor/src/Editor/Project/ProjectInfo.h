#pragma once

#include <string>

namespace Editor
{
    struct ProjectConfig
    {
        std::string name;
        std::string version;
        std::string startScene;
        std::string assetDirectory;
        std::string sourceDirectory;
    };

    class ProjectInfo
    {
    public:
        ProjectInfo() = default;

        const ProjectConfig& GetConfig() const { return _config; }
        const std::string& GetProjectDir() const { return _projectDir; }
        const std::string& GetProjectFilePath() const { return _projectFilePath; }

        bool LoadFromPath(const std::string& path);
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