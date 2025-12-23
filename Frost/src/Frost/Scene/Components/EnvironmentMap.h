#pragma once

#include <variant>
#include <string>
#include <array>
#include <filesystem>

#include "Frost/Scene/ECS/Component.h"

namespace Frost::Component
{
    struct EnvironmentMapSourceCubemap
    {
        std::filesystem::path filepath;
    };

    struct EnvironmentMapSource6Files
    {
        // Order: +X (Right), -X (Left), +Y (Up), -Y (Down), +Z (Forward), -Z (Backward)
        std::array<std::filesystem::path, 6> faceFilepaths;
    };

    using EnvironmentMapConfig = std::variant<EnvironmentMapSourceCubemap, EnvironmentMapSource6Files>;

    enum class EnvironmentMapType
    {
        Cubemap = 0,
        SixFiles = 1
    };

    struct EnvironmentMap : public Component
    {
        EnvironmentMapConfig config = EnvironmentMapSourceCubemap{};

        float intensity = 1.0f;

        EnvironmentMap() = default;
        EnvironmentMap(const EnvironmentMapConfig& cfg) : config(cfg) {}

        EnvironmentMapType GetType() const { return static_cast<EnvironmentMapType>(config.index()); }
    };
} // namespace Frost::Component