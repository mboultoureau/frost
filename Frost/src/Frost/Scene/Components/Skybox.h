#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Debugging/Assert.h"

#include <string>
#include <vector>
#include <memory>

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

		Skybox(const std::array<std::string, 6>& faces)
			: faceFilePaths(faces), cubemapTexture(nullptr)
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
	};
}