#pragma once

#include "Frost/Utils/UUID.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Editor
{
    struct AssetUUIDTag
    {
    };
    using AssetUUID = Frost::UUID<AssetUUIDTag>;

    struct AssetMetadata
    {
        AssetUUID Handle;
        std::string Type;
    };

    class MetadataManager
    {
    public:
        static void EnsureMetadata(const std::filesystem::path& assetPath)
        {
            std::filesystem::path metaPath = assetPath.string() + ".meta";

            if (!std::filesystem::exists(metaPath))
            {
                CreateMetadata(assetPath, metaPath);
            }
        }

        static AssetUUID GetUUID(const std::filesystem::path& assetPath)
        {
            std::filesystem::path metaPath = assetPath.string() + ".meta";
            if (!std::filesystem::exists(metaPath))
            {
                return AssetUUID(static_cast<AssetUUID::ValueType>(0));
            }

            try
            {
                YAML::Node data = YAML::LoadFile(metaPath.string());
                AssetUUID::ValueType id = data["GUID"].as<AssetUUID::ValueType>();
                return AssetUUID(id);
            }
            catch (...)
            {
                return AssetUUID(static_cast<AssetUUID::ValueType>(0));
            }
        }

    private:
        static void CreateMetadata(const std::filesystem::path& asset, const std::filesystem::path& meta)
        {
            AssetUUID newID;

            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "GUID" << YAML::Value << static_cast<uint64_t>(newID.value());
            out << YAML::Key << "OriginalFile" << YAML::Value << asset.filename().string();
            out << YAML::EndMap;

            std::ofstream fout(meta);
            fout << out.c_str();
        }
    };
} // namespace Editor