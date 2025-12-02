#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Asset/AssetManager.h"

#include <memory>
#include <string>
#include <vector>

namespace Frost::Component
{
    struct Skybox : public Component
    {
        /*
         * Face order:
         * - Right: +X
         * - Left: -X
         * - Top: +Y
         * - Bottom: -Y
         * - Front: +Z
         * - Back: -Z
         */
        std::array<std::string, 6> faceFilePaths;
        std::shared_ptr<Texture> cubemapTexture;

        Skybox(const std::array<std::string, 6>& faces) : faceFilePaths(faces), cubemapTexture(nullptr)
        {
            // Check if all face file paths are provided
            for (const auto& path : faceFilePaths)
            {
                FT_ENGINE_ASSERT(!path.empty(), "All 6 face file paths must be provided for the cubemap.");
            }

            TextureConfig cubemapConfig;

            cubemapConfig.path = faceFilePaths[0];
            cubemapConfig.debugName = "Skybox";

            cubemapConfig.layout = TextureLayout::CUBEMAP;
            cubemapConfig.faceFilePaths = faceFilePaths;

            cubemapTexture = AssetManager::LoadAsset(cubemapConfig.path, cubemapConfig);
        }

        Skybox(const std::string& unfoldedFacePath) : faceFilePaths({}), cubemapTexture(nullptr)
        {
            FT_ENGINE_ASSERT(!unfoldedFacePath.empty(), "The unfolded cubemap file path must be provided.");

            TextureConfig cubemapConfig;

            cubemapConfig.path = unfoldedFacePath;
            cubemapConfig.debugName = "SkyboxUnfolded";

            cubemapConfig.layout = TextureLayout::CUBEMAP;
            cubemapConfig.isUnfoldedCubemap = true;

            cubemapTexture = AssetManager::LoadAsset(cubemapConfig.path, cubemapConfig);
        }
    };
} // namespace Frost::Component