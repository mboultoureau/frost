#pragma once

#include "Editor/Project/ProjectInfo.h"

#include <string>

namespace Editor
{
    class PhysicsCodeGenerator
    {
    public:
        static bool Generate(const ProjectConfig& config, const std::string& outputDirectory);

    private:
        static std::string _SanitizeName(const std::string& name);
        static std::string _GenerateHeaderContent(const ProjectConfig& config);
        static std::string _GenerateSourceContent(const ProjectConfig& config);
    };
} // namespace Editor