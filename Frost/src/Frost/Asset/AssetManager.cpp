#include "Frost/Asset/AssetManager.h"

namespace Frost
{
    std::map<Asset::Path, std::shared_ptr<Asset>> AssetManager::_loadedAssets;

    void AssetManager::Shutdown()
    {
        _loadedAssets.clear();
    }
} // namespace Frost
