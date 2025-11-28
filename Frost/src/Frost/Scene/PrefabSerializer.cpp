#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/SerializerUtils.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Debugging/Logger.h"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <map>

using namespace Frost;
using namespace Frost::Component;

namespace Frost
{
    void PrefabSerializer::CreatePrefab(GameObject rootEntity, const std::filesystem::path& destinationPath)
    {
        if (!rootEntity)
            return;

        std::vector<GameObject> entitiesToSerialize;
        _FlattenHierarchy(rootEntity, entitiesToSerialize);

        std::filesystem::path yamlPath = destinationPath;
        yamlPath.replace_extension(".prefab");

        std::filesystem::path binPath = destinationPath;
        binPath.replace_extension(".bin");

        _SerializeToYaml(entitiesToSerialize, yamlPath);
        _SerializeToBinary(entitiesToSerialize, binPath);
    }

    void PrefabSerializer::_FlattenHierarchy(GameObject root, std::vector<GameObject>& outList)
    {
        outList.push_back(root);

        if (root.HasComponent<Component::Relationship>())
        {
            auto children = root.GetChildren();
            for (auto child : children)
            {
                _FlattenHierarchy(child, outList);
            }
        }
    }

    void PrefabSerializer::_SerializeToYaml(const std::vector<GameObject>& entities, const std::filesystem::path& path)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Prefab" << YAML::Value << "FrostEngine";
        out << YAML::Key << "Entities" << YAML::BeginSeq;

        std::map<entt::entity, uint32_t> entityToIndex;
        for (size_t i = 0; i < entities.size(); ++i)
            entityToIndex[entities[i].GetHandle()] = static_cast<uint32_t>(i);

        for (auto& entity : entities)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "LocalID" << YAML::Value << entityToIndex[entity.GetHandle()];

            // Relationship is more complex due to ParentID
            if (entity.HasComponent<Component::Relationship>())
            {
                auto& rel = entity.GetComponent<Component::Relationship>();
                out << YAML::Key << "ParentID";
                if (rel.parent != entt::null && entityToIndex.count(rel.parent))
                {
                    out << YAML::Value << entityToIndex[rel.parent];
                }
                else
                {
                    out << YAML::Value << -1;
                }
            }

            for (const auto& serializer : SerializationSystem::GetAllSerializers())
            {
                if (serializer.Name == "Relationship")
                {
                    continue;
                }

                if (serializer.HasComponent(entity))
                {
                    out << YAML::Key << serializer.Name;
                    out << YAML::BeginMap;
                    serializer.SerializeYaml(out, entity);
                    out << YAML::EndMap;
                }
            }

            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(path);
        fout << out.c_str();
    }

    void PrefabSerializer::_SerializeToBinary(const std::vector<GameObject>& entities,
                                              const std::filesystem::path& path)
    {
        std::ofstream out(path, std::ios::binary);
        if (!out.is_open())
            return;

        // Header
        const char header[] = "FROST_BIN";
        out.write(header, sizeof(header));
        uint32_t count = (uint32_t)entities.size();
        out.write((char*)&count, sizeof(uint32_t));

        // Map Index
        std::map<entt::entity, uint32_t> entityToIndex;
        for (size_t i = 0; i < entities.size(); ++i)
            entityToIndex[entities[i].GetHandle()] = (uint32_t)i;

        for (auto& entity : entities)
        {
            uint32_t localID = entityToIndex[entity.GetHandle()];
            out.write((char*)&localID, sizeof(uint32_t));

            bool hasRelationship = entity.HasComponent<Component::Relationship>();
            out.write((char*)&hasRelationship, sizeof(bool));

            int32_t parentID = -1;
            if (hasRelationship)
            {
                auto& rel = entity.GetComponent<Component::Relationship>();
                if (rel.parent != entt::null && entityToIndex.count(rel.parent))
                {
                    parentID = (int32_t)entityToIndex[rel.parent];
                }
            }
            out.write((char*)&parentID, sizeof(int32_t));

            for (const auto& serializer : SerializationSystem::GetAllSerializers())
            {
                if (serializer.Name == "Relationship")
                {
                    continue;
                }

                if (serializer.HasComponent(entity))
                {
                    out.write((char*)&serializer.ID, sizeof(uint32_t));
                    serializer.SerializeBinary(out, entity);
                }
            }

            uint32_t endMarker = 0;
            out.write((char*)&endMarker, sizeof(uint32_t));
        }
    }

    GameObject PrefabSerializer::Instantiate(Scene* scene, const std::filesystem::path& path)
    {
        std::string extension = path.extension().string();

        if (extension == ".prefab")
        {
            std::ifstream stream(path);
            if (!stream.is_open())
            {
                return GameObject();
            }

            std::stringstream strStream;
            strStream << stream.rdbuf();

            YAML::Node data = YAML::Load(strStream.str());
            if (!data["Prefab"] || !data["Entities"])
            {
                return GameObject();
            }

            auto entitiesNode = data["Entities"];
            std::map<uint32_t, GameObject> localIdToEntity;
            std::vector<std::pair<GameObject, int32_t>> pendingParents;

            for (auto entityNode : entitiesNode)
            {
                uint32_t localID = entityNode["LocalID"].as<uint32_t>();
                entt::entity handle = scene->GetRegistry().create();
                GameObject go(handle, scene);
                localIdToEntity[localID] = go;

                if (entityNode["Meta"])
                    go.AddComponent<Meta>();

                if (entityNode["ParentID"])
                {
                    go.AddComponent<Relationship>();
                    int32_t parentID = entityNode["ParentID"].as<int32_t>();
                    pendingParents.push_back({ go, parentID });
                }

                for (const auto& serializer : SerializationSystem::GetAllSerializers())
                {
                    if (serializer.Name == "Relationship")
                    {
                        continue;
                    }

                    if (entityNode[serializer.Name])
                    {
                        serializer.AddComponent(go);
                        serializer.DeserializeYaml(entityNode[serializer.Name], go);
                    }
                }
            }

            // Hierarchy reconstruction
            GameObject root = GameObject::InvalidId;
            for (auto& [go, parentID] : pendingParents)
            {
                if (parentID != -1 && localIdToEntity.find(parentID) != localIdToEntity.end())
                {
                    go.SetParent(localIdToEntity[parentID]);
                }
                else
                {
                    root = go;
                }
            }

            if (!root && !localIdToEntity.empty())
            {
                root = localIdToEntity[0];
            }

            return root;
        }
        else if (extension == ".bin")
        {
            std::ifstream in(path, std::ios::binary);
            if (!in.is_open())
            {
                return GameObject();
            }

            char header[12]; // "FROST_BIN" + null
            in.read(header, 10);

            uint32_t entityCount;
            in.read((char*)&entityCount, sizeof(uint32_t));

            std::map<uint32_t, GameObject> localIdToEntity;
            std::vector<std::pair<GameObject, int32_t>> pendingParents;

            for (uint32_t i = 0; i < entityCount; i++)
            {
                uint32_t localID;
                in.read((char*)&localID, sizeof(uint32_t));

                entt::entity handle = scene->GetRegistry().create();
                GameObject go(handle, scene);
                localIdToEntity[localID] = go;

                bool hasRelationship;
                in.read((char*)&hasRelationship, sizeof(bool));

                int32_t parentID;
                in.read((char*)&parentID, sizeof(int32_t));

                if (hasRelationship)
                {
                    go.AddComponent<Component::Relationship>();
                }
                pendingParents.push_back({ go, parentID });

                while (true)
                {
                    uint32_t componentID;
                    in.read((char*)&componentID, sizeof(uint32_t));

                    if (componentID == 0)
                    {
                        break;
                    }

                    auto* serializer = SerializationSystem::GetSerializerByID(componentID);
                    if (serializer)
                    {
                        serializer->AddComponent(go);
                        serializer->DeserializeBinary(in, go);
                    }
                    else
                    {
                        FT_ENGINE_ERROR("Unknown Component ID in Prefab: {0}. Binary stream corrupted.", componentID);
                        return GameObject();
                    }
                }
            }

            GameObject root = GameObject::InvalidId;
            for (auto& [go, parentID] : pendingParents)
            {
                if (parentID != -1 && localIdToEntity.count(parentID))
                {
                    go.SetParent(localIdToEntity[parentID]);
                }
                else
                {
                    root = go;
                }
            }
            if (!root && !localIdToEntity.empty())
            {
                root = localIdToEntity[0];
            }
            return root;
        }

        return GameObject();
    }
} // namespace Frost