#include "Frost/Scene/SceneSerializer.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/SerializerUtils.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Prefab.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Systems/ScriptableSystem.h"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <unordered_map>

namespace Frost
{
    SceneSerializer::SceneSerializer(Scene* scene) : m_Scene(scene) {}

    bool SceneSerializer::_IsDescendantOfPrefab(GameObject go)
    {
        if (!go || !go.HasComponent<Component::Relationship>())
        {
            return false;
        }

        auto parentHandle = go.GetComponent<Component::Relationship>().parent;
        while (parentHandle != entt::null)
        {
            GameObject parentGo(parentHandle, go.GetScene());
            if (!parentGo)
            {
                break;
            }

            if (parentGo.HasComponent<Component::Prefab>())
            {
                return true;
            }

            if (parentGo.HasComponent<Component::Relationship>())
            {
                parentHandle = parentGo.GetComponent<Component::Relationship>().parent;
            }
            else
            {
                break;
            }
        }

        return false;
    }

    bool SceneSerializer::Serialize(const std::filesystem::path& filepath)
    {
        std::string extension = filepath.extension().string();
        if (extension == ".yaml" || extension == ".scene")
        {
            return _SerializeToYaml(filepath);
        }
        else if (extension == ".bin")
        {
            return _SerializeToBinary(filepath);
        }
        else
        {
            FT_ENGINE_WARN("Unsupported scene file format for serialization: {0}", extension);
            return false;
        }
    }

    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::string extension = filepath.extension().string();
        if (extension == ".yaml" || extension == ".scene")
        {
            return _DeserializeFromYaml(filepath);
        }
        else if (extension == ".bin")
        {
            return _DeserializeFromBinary(filepath);
        }

        FT_ENGINE_ERROR("Unsupported scene file format for deserialization: {0}", extension);
        return false;
    }

    bool SceneSerializer::_SerializeToYaml(const std::filesystem::path& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();

        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        auto view = m_Scene->GetRegistry().view<entt::entity>();
        for (auto entityID : view)
        {
            GameObject go(entityID, m_Scene);
            if (!go)
                continue;

            if (auto* meta = go.TryGetComponent<Component::Meta>())
            {
                if (meta->name.rfind("__EDITOR__", 0) == 0)
                    continue;
            }

            if (_IsDescendantOfPrefab(go))
            {
                continue;
            }

            out << YAML::BeginMap;
            out << YAML::Key << "Entity" << YAML::Value << static_cast<uint32_t>(entityID);

            for (const auto& serializer : SerializationSystem::GetAllSerializers())
            {
                if (serializer.Name == "Relationship")
                    continue;

                if (serializer.HasComponent(go))
                {
                    out << YAML::Key << serializer.Name;
                    out << YAML::BeginMap;
                    serializer.SerializeYaml(out, go);
                    out << YAML::EndMap;
                }
            }

            int32_t parentID = -1;
            if (go.HasComponent<Component::Relationship>())
            {
                entt::entity parentHandle = go.GetComponent<Component::Relationship>().parent;
                if (parentHandle != entt::null)
                {
                    if (!_IsDescendantOfPrefab(GameObject(parentHandle, m_Scene)))
                    {
                        parentID = static_cast<int32_t>(parentHandle);
                    }
                }
            }
            out << YAML::Key << "Parent" << YAML::Value << parentID;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        if (!fout.is_open())
        {
            FT_ENGINE_ERROR("Failed to open file for writing: {}", filepath.string());
            return false;
        }

        fout << out.c_str();
        fout.close();

        return true;
    }

    bool SceneSerializer::_DeserializeFromYaml(const std::filesystem::path& filepath)
    {
        std::ifstream stream(filepath);
        if (!stream.is_open())
        {
            FT_ENGINE_ERROR("Failed to open scene file: {0}", filepath.string());
            return false;
        }
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Scene"])
        {
            FT_ENGINE_WARN("Invalid scene file format: {0}", filepath.string());
            return false;
        }

        m_Scene->SetName(data["Scene"].as<std::string>());

        auto entities = data["Entities"];
        if (entities)
        {
            m_Scene->Clear();
            std::unordered_map<uint32_t, entt::entity> entityMap;

            for (auto entityNode : entities)
            {
                uint32_t oldID = entityNode["Entity"].as<uint32_t>();
                GameObject newGo = m_Scene->CreateGameObject("TempName");
                entityMap[oldID] = newGo.GetHandle();
            }

            for (auto entityNode : entities)
            {
                uint32_t oldID = entityNode["Entity"].as<uint32_t>();
                GameObject go(entityMap.at(oldID), m_Scene);

                for (const auto& serializer : SerializationSystem::GetAllSerializers())
                {
                    if (serializer.Name == "Relationship")
                        continue;

                    if (entityNode[serializer.Name])
                    {
                        if (!serializer.HasComponent(go))
                            serializer.AddComponent(go);
                        serializer.DeserializeYaml(entityNode[serializer.Name], go);
                    }
                }
            }

            for (auto entityNode : entities)
            {
                uint32_t oldID = entityNode["Entity"].as<uint32_t>();
                int32_t parentID = entityNode["Parent"].as<int32_t>(-1);

                if (parentID != -1)
                {
                    GameObject childGo(entityMap.at(oldID), m_Scene);
                    if (entityMap.count(static_cast<uint32_t>(parentID)))
                    {
                        GameObject parentGo(entityMap.at(static_cast<uint32_t>(parentID)), m_Scene);
                        childGo.SetParent(parentGo);
                    }
                }
            }
        }

        std::vector<entt::entity> prefabInstances;
        auto prefabView = m_Scene->GetRegistry().view<Component::Prefab>();
        for (auto entity : prefabView)
        {
            prefabInstances.push_back(entity);
        }

        for (auto entityID : prefabInstances)
        {
            GameObject placeholder(entityID, m_Scene);
            if (!placeholder)
                continue;

            placeholder.DestroyAllChildren();

            const auto& prefabComponent = placeholder.GetComponent<Component::Prefab>();
            const auto& assetPath = prefabComponent.assetPath;

            if (!assetPath.empty())
            {
                GameObject prefabRoot = PrefabSerializer::Instantiate(m_Scene, assetPath);

                if (prefabRoot)
                {
                    prefabRoot.SetParent(placeholder);
                }
                else
                {
                    FT_ENGINE_WARN("Impossible d'instancier le prefab '{0}' pour l'entité placeholder.",
                                   assetPath.string());
                }
            }
        }

        return true;
    }

    bool SceneSerializer::_SerializeToBinary(const std::filesystem::path& filepath)
    {
        std::ofstream out(filepath, std::ios::binary);
        if (!out.is_open())
        {
            FT_ENGINE_CRITICAL("Failed to open file for writing: {}", filepath.string());
            return false;
        }

        const char header[] = "FROST_SCENE_BIN";
        out.write(header, sizeof(header) - 1);
        WriteBinaryString(out, m_Scene->GetName());

        std::vector<entt::entity> entitiesToSerialize;
        auto allEntitiesView = m_Scene->GetRegistry().view<entt::entity>();
        for (auto entity : allEntitiesView)
        {
            GameObject go(entity, m_Scene);
            if (auto* meta = go.TryGetComponent<Component::Meta>())
            {
                if (meta->name.rfind("__EDITOR__", 0) == 0)
                    continue;
            }

            if (_IsDescendantOfPrefab(go))
            {
                continue;
            }

            entitiesToSerialize.push_back(entity);
        }

        uint32_t count = static_cast<uint32_t>(entitiesToSerialize.size());
        out.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

        for (auto& entity : entitiesToSerialize)
        {
            GameObject go(entity, m_Scene);
            uint32_t entityId = static_cast<uint32_t>(entity);
            out.write(reinterpret_cast<const char*>(&entityId), sizeof(uint32_t));

            int32_t parentId = -1;
            if (go.HasComponent<Component::Relationship>())
            {
                auto parentHandle = go.GetComponent<Component::Relationship>().parent;
                if (parentHandle != entt::null)
                {
                    parentId = static_cast<int32_t>(parentHandle);
                }
            }
            out.write(reinterpret_cast<const char*>(&parentId), sizeof(int32_t));

            for (const auto& serializer : SerializationSystem::GetAllSerializers())
            {
                if (serializer.Name == "Relationship")
                    continue;

                if (serializer.HasComponent(go))
                {
                    out.write(reinterpret_cast<const char*>(&serializer.ID), sizeof(uint32_t));
                    serializer.SerializeBinary(out, go);
                }
            }
            uint32_t endMarker = 0;
            out.write(reinterpret_cast<const char*>(&endMarker), sizeof(uint32_t));
        }

        out.close();
        return true;
    }

    bool SceneSerializer::_DeserializeFromBinary(const std::filesystem::path& filepath)
    {
        // Print absolute path for debugging
        auto absPath = std::filesystem::absolute(filepath);
        FT_ENGINE_INFO("Deserializing scene from: {0}", absPath.string());

        std::ifstream in(filepath, std::ios::binary);
        if (!in.is_open())
        {
            FT_ENGINE_ERROR("Failed to open scene file: {0}", filepath.string());
            return false;
        }

        char header[16] = { 0 };
        in.read(header, 15);
        if (std::string(header) != "FROST_SCENE_BIN")
            return false;

        m_Scene->SetName(ReadBinaryString(in));
        m_Scene->Clear();

        std::unordered_map<uint32_t, entt::entity> entityMap;
        std::vector<std::pair<uint32_t, int32_t>> parentChildMap; // {child_old_id, parent_old_id}

        uint32_t entityCount;
        in.read(reinterpret_cast<char*>(&entityCount), sizeof(uint32_t));

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            uint32_t oldID;
            in.read(reinterpret_cast<char*>(&oldID), sizeof(uint32_t));
            GameObject newGo = m_Scene->CreateGameObject("TempName");
            entityMap[oldID] = newGo.GetHandle();

            int32_t parentID;
            in.read(reinterpret_cast<char*>(&parentID), sizeof(int32_t));
            if (parentID != -1)
                parentChildMap.push_back({ oldID, parentID });

            while (true)
            {
                uint32_t componentID;
                in.read(reinterpret_cast<char*>(&componentID), sizeof(uint32_t));
                if (componentID == 0)
                    break; // Fin des composants pour cette entité

                auto* serializer = SerializationSystem::GetSerializerByID(componentID);
                if (serializer)
                {
                    if (!serializer->HasComponent(newGo))
                        serializer->AddComponent(newGo);
                    serializer->DeserializeBinary(in, newGo);
                }
                else
                {
                    FT_ENGINE_ERROR("Unknown component ID {0} in scene file. Deserialization may fail.", componentID);
                    return false;
                }
            }
        }

        for (const auto& pair : parentChildMap)
        {
            GameObject childGo(entityMap.at(pair.first), m_Scene);
            uint32_t parentOldId = static_cast<uint32_t>(pair.second);
            if (entityMap.count(parentOldId))
            {
                GameObject parentGo(entityMap.at(parentOldId), m_Scene);
                childGo.SetParent(parentGo);
            }
            else
            {
                FT_ENGINE_WARN("Parent entity with old ID {0} not found for child {1}.", parentOldId, pair.first);
            }
        }

        std::vector<entt::entity> prefabInstances;
        auto prefabView = m_Scene->GetRegistry().view<Component::Prefab>();
        for (auto entity : prefabView)
        {
            prefabInstances.push_back(entity);
        }

        for (auto entityID : prefabInstances)
        {
            GameObject placeholder(entityID, m_Scene);
            if (!placeholder)
                continue;

            const auto& prefabComponent = placeholder.GetComponent<Component::Prefab>();
            const auto& assetPath = prefabComponent.assetPath;

            if (!assetPath.empty())
            {
                std::filesystem::path binPath = assetPath;
                binPath.replace_extension(".bin");

                GameObject prefabRoot =
                    PrefabSerializer::Instantiate(m_Scene, std::filesystem::exists(binPath) ? binPath : assetPath);

                if (prefabRoot)
                {
                    prefabRoot.SetParent(placeholder);
                }
                else
                {
                    FT_ENGINE_WARN("Impossible d'instancier le prefab '{0}' pour l'entité placeholder.",
                                   assetPath.string());
                }
            }
        }

        if (auto* scriptSystem = m_Scene->GetSystem<ScriptableSystem>())
        {
            scriptSystem->OnScriptsReloaded();
        }

        return true;
    }
} // namespace Frost