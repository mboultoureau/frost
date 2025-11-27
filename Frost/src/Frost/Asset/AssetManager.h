#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"

#ifdef FT_PLATFORM_WINDOWS
	#include "Frost/Renderer/DX11/TextureDX11.h"
#endif

#include <memory>
#include <map>

namespace Frost
{
	class AssetManager
	{
	public:

		static void Shutdown();

		// Generic asset loading function
		template<typename T, typename... Args>
		static std::shared_ptr<T> LoadAsset(const Asset::Path& path, Args&&... args) requires (!std::is_same_v<T, Texture>)
		{
			static_assert(std::is_base_of<Asset, T>::value, "T must be derived from Frost::Asset");

			// Check if asset is already loaded
			auto it = _loadedAssets.find(path);
			if (it != _loadedAssets.end())
			{
				return std::static_pointer_cast<T>(it->second);
			}

			// Load new asset
			try
			{
				std::shared_ptr<T> asset = std::make_shared<T>(std::forward<Args>(args)...);
				_loadedAssets[path] = asset;
				return asset;
			}
			catch (const std::exception& e)
			{
				FT_ENGINE_ERROR("Failed to load asset '{}': {}", path, e.what());
				FT_ENGINE_ASSERT(false, "Failed to load asset");
				return nullptr;
			}
			catch (...)
			{
				FT_ENGINE_ERROR("Unknown error while loading asset '{}'", path);
				FT_ENGINE_ASSERT(false, "Failed to load asset");
				return nullptr;
			}
		}

		// Specialized function to get already loaded asset
		static std::shared_ptr<Texture> LoadAsset(const Asset::Path& path, TextureConfig& config)
		{
			auto it = _loadedAssets.find(path);
			if (it != _loadedAssets.end())
			{
				return std::static_pointer_cast<Texture>(it->second);
			}

#ifdef FT_PLATFORM_WINDOWS
			std::shared_ptr<TextureDX11> texture = std::make_shared<TextureDX11>(config);
#else
#error "Texture loading not implemented for this platform"
#endif
			_loadedAssets[path] = texture;
			return texture;
		}

		static std::shared_ptr<Texture> CreateTexture(const TextureConfig& config)
		{
#ifdef FT_PLATFORM_WINDOWS
			TextureConfig cfg = config;
			return std::make_shared<TextureDX11>(cfg);
#else
#error "Texture creation not implemented for this platform"
#endif
		}

	private:
		static std::map<Asset::Path, std::shared_ptr<Asset>> _loadedAssets;
	};
}


