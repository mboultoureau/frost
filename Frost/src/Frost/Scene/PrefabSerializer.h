#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/Scene.h"

#include <filesystem>
#include <string>

namespace Frost
{
    class FROST_API PrefabSerializer
    {
    public:
        static void CreatePrefab(GameObject rootEntity, const std::filesystem::path& destinationPath);
        static GameObject Instantiate(Scene* scene,
                                      const std::filesystem::path& path,
                                      std::set<std::filesystem::path>* instantiationStack = nullptr);

    private:
        static void _SerializeToYaml(GameObject rootEntity,
                                     const std::vector<GameObject>& entities,
                                     const std::filesystem::path& path);
        static void _SerializeToBinary(GameObject rootEntity,
                                       const std::vector<GameObject>& entities,
                                       const std::filesystem::path& path);
        static void _FlattenHierarchy(GameObject root, std::vector<GameObject>& outList);
    };
} // namespace Frost