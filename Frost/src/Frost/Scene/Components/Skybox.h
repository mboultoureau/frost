#pragma once

#include <variant>
#include <string>
#include <array>
#include <filesystem>

#include "Frost/Scene/ECS/Component.h"

namespace Frost::Component
{
    struct SkyboxSourceCubemap
    {
        std::filesystem::path filepath;
    };

    struct SkyboxSource6Files
    {
        // Order: +X (Right), -X (Left), +Y (Up), -Y (Down), +Z (Forward), -Z (Backward)
        std::array<std::filesystem::path, 6> faceFilepaths;
    };

    using SkyboxConfig = std::variant<SkyboxSourceCubemap, SkyboxSource6Files>;

    enum class SkyboxType
    {
        Cubemap = 0,
        SixFiles = 1
    };

    struct Skybox : public Component
    {
        SkyboxConfig config = SkyboxSourceCubemap{};
        float intensity = 1.0f;

        Skybox() = default;
        Skybox(const SkyboxConfig& cfg) : config(cfg) {}

        SkyboxType GetType() const { return static_cast<SkyboxType>(config.index()); }

        void SetType(SkyboxType type)
        {
            if (GetType() == type)
            {
                return;
            }

            switch (type)
            {
                case SkyboxType::Cubemap:
                    config = SkyboxSourceCubemap{};
                    break;
                case SkyboxType::SixFiles:
                    config = SkyboxSource6Files{};
                    break;
            }
        }
    };
} // namespace Frost::Component