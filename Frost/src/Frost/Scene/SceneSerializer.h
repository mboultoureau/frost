#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Scene/Scene.h"

#include <filesystem>

namespace Frost
{
    class FROST_API SceneSerializer
    {
    public:
        SceneSerializer(Scene* scene);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

    private:
        bool _SerializeToYaml(const std::filesystem::path& filepath);
        bool _DeserializeFromYaml(const std::filesystem::path& filepath);

        bool _SerializeToBinary(const std::filesystem::path& filepath);
        bool _DeserializeFromBinary(const std::filesystem::path& filepath);

        bool _IsDescendantOfPrefab(GameObject go);

        Scene* m_Scene;
    };
} // namespace Frost