#pragma once

#include <filesystem>

namespace Frost::Component
{
    struct Prefab
    {
        std::filesystem::path assetPath;

        Prefab() : assetPath{} {}
        Prefab(const std::filesystem::path& path) : assetPath{ path } {}
    };
} // namespace Frost::Component