#pragma once

#include "Frost/Utils/Math/Vector.h"

#include <string>
#include <array>
#include <string_view>
#include <filesystem>

namespace Frost::Component
{
    // File extensions supported
    constexpr const std::array<std::string_view, 4> HEIGHTMAP_EXTENSIONS{ ".png", ".jpg", ".jpeg", ".bmp" };
    constexpr const std::array<std::string_view, 8> MESH_FILE_EXTENSIONS{ ".fbx", ".obj",  ".gltf", ".glb",
                                                                          ".usd", ".usda", ".usdc", ".usdz" };

    struct MeshSourceHeightMap
    {
        std::string texturePath;
        float width = 10.0f;
        float depth = 10.0f;
        float minHeight = 0.0f;
        float maxHeight = 2.0f;
        uint32_t segmentsWidth = 64;
        uint32_t segmentsDepth = 64;
    };

    struct MeshSourceFile
    {
        std::filesystem::path filepath;
    };

    struct MeshSourceCube
    {
        float size = 1.0f;
        Frost::Math::Vector3 segments = { 1.0f, 1.0f, 1.0f };
        float bevelRadius = 0.0f;
    };

    struct MeshSourceSphere
    {
        float radius = 0.5f;
        uint32_t rings = 20;
        uint32_t slices = 20;
    };

    struct MeshSourcePlane
    {
        float width = 1.0f;
        float depth = 1.0f;
    };

    struct MeshSourceCylinder
    {
        float bottomRadius = 1.0f;
        float topRadius = 1.0f;
        float height = 2.0f;
        uint32_t slices = 32;
        uint32_t stacks = 1;
    };
} // namespace Frost::Component