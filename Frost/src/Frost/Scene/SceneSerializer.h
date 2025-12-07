#pragma once

#include "Frost/Scene/Scene.h"
#include <filesystem>

namespace Frost
{
    class SceneSerializer
    {
    public:
        SceneSerializer(Scene* scene);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

    private:
        bool SerializeToYaml(const std::filesystem::path& filepath);
        bool DeserializeFromYaml(const std::filesystem::path& filepath);

        bool SerializeToBinary(const std::filesystem::path& filepath);
        bool DeserializeFromBinary(const std::filesystem::path& filepath);

        Scene* m_Scene;
    };
} // namespace Frost