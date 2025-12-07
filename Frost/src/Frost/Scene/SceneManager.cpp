#include "Frost/Scene/SceneManager.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Scene/SceneSerializer.h"

namespace Frost
{
    std::unordered_map<std::string, std::shared_ptr<Scene>> SceneManager::_loadedScenes;

    std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name)
    {
        if (_loadedScenes.count(name))
        {
            FT_ENGINE_WARN("SceneManager::CreateScene: A scene named '{0}' already exists.", name);
            return _loadedScenes[name];
        }

        auto newScene = std::make_shared<Scene>(name);
        _loadedScenes[name] = newScene;
        return newScene;
    }

    std::shared_ptr<Scene> SceneManager::LoadSceneFromFile(const std::string& filepath)
    {
        auto newScene = std::make_shared<Scene>("Temporary Loading Scene");

        SceneSerializer serializer(newScene.get());

        if (serializer.Deserialize(filepath))
        {
            const std::string& sceneName = newScene->GetName();
            if (_loadedScenes.count(sceneName))
            {
                FT_ENGINE_INFO("SceneManager: Unloading existing scene named '{0}' to reload.", sceneName);
            }

            FT_ENGINE_INFO("Scene '{0}' loaded successfully from {1}", sceneName, filepath);
            _loadedScenes[sceneName] = newScene;
            return newScene;
        }
        else
        {
            FT_ENGINE_ERROR("Failed to load scene from file: {0}", filepath);
            return nullptr;
        }
    }

    std::shared_ptr<Scene> SceneManager::GetScene(const std::string& name)
    {
        auto it = _loadedScenes.find(name);
        if (it != _loadedScenes.end())
        {
            return it->second;
        }

        return nullptr;
    }

    void SceneManager::UnloadScene(const std::string& name)
    {
        auto it = _loadedScenes.find(name);
        if (it != _loadedScenes.end())
        {
            _loadedScenes.erase(it);
            FT_ENGINE_INFO("Scene '{0}' has been unloaded.", name);
        }
    }

    void SceneManager::Shutdown()
    {
        _loadedScenes.clear();
    }
} // namespace Frost
