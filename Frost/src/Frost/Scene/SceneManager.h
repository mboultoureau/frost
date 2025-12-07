#pragma once

#include "Frost/Scene/Scene.h"

#include <memory>
#include <vector>
#include <unordered_map>

namespace Frost
{
    class SceneManager
    {
    public:
        static std::shared_ptr<Scene> CreateScene(const std::string& name = "Scene");
        static std::shared_ptr<Scene> LoadSceneFromFile(const std::string& filepath);
        static std::shared_ptr<Scene> GetScene(const std::string& name);
        static void UnloadScene(const std::string& name);
        static void Shutdown();

    private:
        static std::unordered_map<std::string, std::shared_ptr<Scene>> _loadedScenes;
    };
} // namespace Frost