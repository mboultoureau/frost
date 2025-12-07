#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/SerializerUtils.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Prefab.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Core/Application.h"
#include "Frost/Debugging/Logger.h"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <map>
#include <sstream>

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

        if (root.HasComponent<Component::Prefab>())
        {
            return;
        }

        if (root.HasComponent<Component::Relationship>())
        {
            const auto& rel = root.GetComponent<Component::Relationship>();
            auto currentChildHandle = rel.firstChild;

            while (currentChildHandle != entt::null)
            {
                GameObject child(currentChildHandle, root.GetScene());
                _FlattenHierarchy(child, outList);

                if (child.HasComponent<Component::Relationship>())
                {
                    currentChildHandle = child.GetComponent<Component::Relationship>().nextSibling;
                }
                else
                {
                    currentChildHandle = entt::null;
                }
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

            if (entity.HasComponent<Component::Relationship>())
            {
                auto& rel = entity.GetComponent<Component::Relationship>();
                out << YAML::Key << "ParentID";
                if (rel.parent != entt::null && entityToIndex.count(rel.parent))
                    out << YAML::Value << entityToIndex[rel.parent];
                else
                    out << YAML::Value << -1;
            }

            if (entity.HasComponent<Component::Prefab>() && !entity.GetComponent<Component::Prefab>().assetPath.empty())
            {
                const char* componentsToSerialize[] = { "Meta", "Transform", "Prefab" };
                for (const auto& name : componentsToSerialize)
                {
                    for (const auto& serializer : SerializationSystem::GetAllSerializers())
                    {
                        if (serializer.Name == name && serializer.HasComponent(entity))
                        {
                            out << YAML::Key << serializer.Name;
                            out << YAML::BeginMap;
                            serializer.SerializeYaml(out, entity);
                            out << YAML::EndMap;
                            break;
                        }
                    }
                }
            }
            else // Sinon, sérialisation normale
            {
                for (const auto& serializer : SerializationSystem::GetAllSerializers())
                {
                    if (serializer.Name == "Relationship")
                        continue;

                    if (serializer.HasComponent(entity))
                    {
                        out << YAML::Key << serializer.Name;
                        out << YAML::BeginMap;
                        serializer.SerializeYaml(out, entity);
                        out << YAML::EndMap;
                    }
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

        const char header[] = "FROST_BIN";
        out.write(header, sizeof(header) - 1);
        uint32_t count = (uint32_t)entities.size();
        out.write((char*)&count, sizeof(uint32_t));

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
                    parentID = (int32_t)entityToIndex[rel.parent];
            }
            out.write((char*)&parentID, sizeof(int32_t));

            if (entity.HasComponent<Component::Prefab>() && !entity.GetComponent<Component::Prefab>().assetPath.empty())
            {
                const char* componentsToSerialize[] = { "Meta", "Transform", "Prefab" };
                for (const auto& name : componentsToSerialize)
                {
                    for (const auto& serializer : SerializationSystem::GetAllSerializers())
                    {
                        if (serializer.Name == name && serializer.HasComponent(entity))
                        {
                            out.write((char*)&serializer.ID, sizeof(uint32_t));
                            serializer.SerializeBinary(out, entity);
                            break;
                        }
                    }
                }
            }
            else
            {
                for (const auto& serializer : SerializationSystem::GetAllSerializers())
                {
                    if (serializer.Name == "Relationship")
                        continue;
                    if (serializer.HasComponent(entity))
                    {
                        out.write((char*)&serializer.ID, sizeof(uint32_t));
                        serializer.SerializeBinary(out, entity);
                    }
                }
            }

            uint32_t endMarker = 0;
            out.write((char*)&endMarker, sizeof(uint32_t));
        }
    }

    GameObject PrefabSerializer::Instantiate(Scene* scene,
                                             const std::filesystem::path& path,
                                             std::set<std::filesystem::path>* instantiationStack)
    {
        std::filesystem::path absolutePath = path;
        if (path.is_relative())
        {
            absolutePath = Application::GetProjectDirectory() / path;
        }
        absolutePath = std::filesystem::weakly_canonical(absolutePath);

        std::set<std::filesystem::path> localStack;
        if (!instantiationStack)
            instantiationStack = &localStack;

        if (instantiationStack->count(absolutePath))
        {
            FT_ENGINE_ERROR("Recursive prefab detected! Path '{0}' is already in the instantiation stack. Aborting.",
                            absolutePath.string());
            return GameObject();
        }
        instantiationStack->insert(absolutePath);

        GameObject root;
        std::string extension = absolutePath.extension().string();

        if (extension == ".prefab")
        {
            std::ifstream stream(absolutePath);
            if (!stream.is_open())
            {
                FT_ENGINE_ERROR("Failed to open prefab file: {0}", absolutePath.string());
                instantiationStack->erase(absolutePath);
                return GameObject();
            }

            std::stringstream strStream;
            strStream << stream.rdbuf();
            YAML::Node data = YAML::Load(strStream.str());

            if (!data["Prefab"] || !data["Entities"])
            {
                FT_ENGINE_WARN("Invalid prefab file format: {0}", absolutePath.string());
                instantiationStack->erase(absolutePath);
                return GameObject();
            }

            auto entitiesNode = data["Entities"];
            std::map<uint32_t, GameObject> localIdToEntity;
            std::vector<std::pair<GameObject, int32_t>> pendingParents;

            for (auto entityNode : entitiesNode)
            {
                uint32_t localID = entityNode["LocalID"].as<uint32_t>();
                GameObject go;

                if (entityNode["Prefab"])
                {
                    std::filesystem::path nestedPath = entityNode["Prefab"]["AssetPath"].as<std::string>();
                    std::filesystem::path absoluteNestedPath = nestedPath;

                    if (nestedPath.is_relative())
                    {
                        absoluteNestedPath = Application::GetProjectDirectory() / nestedPath;
                    }

                    if (nestedPath.empty() || !std::filesystem::exists(absoluteNestedPath))
                    {
                        FT_ENGINE_WARN("Nested prefab path '{0}' (resolved to '{1}') is invalid or file does not "
                                       "exist. Creating empty entity instead.",
                                       nestedPath.string(),
                                       absoluteNestedPath.string());
                        go = GameObject(scene->GetRegistry().create(), scene);
                    }
                    else
                    {
                        go = Instantiate(scene, absoluteNestedPath, instantiationStack);
                    }
                }
                else
                {
                    go = GameObject(scene->GetRegistry().create(), scene);
                }

                if (!go)
                    continue;
                localIdToEntity[localID] = go;

                for (const auto& serializer : SerializationSystem::GetAllSerializers())
                {
                    if (entityNode[serializer.Name])
                    {
                        serializer.AddComponent(go);
                        serializer.DeserializeYaml(entityNode[serializer.Name], go);
                    }
                }

                if (entityNode["ParentID"])
                {
                    if (!go.HasComponent<Relationship>())
                        go.AddComponent<Relationship>();
                    int32_t parentID = entityNode["ParentID"].as<int32_t>();
                    pendingParents.push_back({ go, parentID });
                }
            }

            for (auto& [go, parentID] : pendingParents)
            {
                if (parentID != -1 && localIdToEntity.count(parentID))
                    go.SetParent(localIdToEntity[parentID]);
                else
                    root = go;
            }

            if (!root && !localIdToEntity.empty())
                root = localIdToEntity.begin()->second;
        }
        else if (extension == ".bin")
        {
            std::ifstream in(absolutePath, std::ios::binary);
            if (!in.is_open())
            {
                FT_ENGINE_ERROR("Failed to open binary prefab file: {0}", absolutePath.string());
                instantiationStack->erase(absolutePath);
                return GameObject();
            }

            char header[10] = { 0 };
            in.read(header, 9);
            if (std::string(header) != "FROST_BIN")
            {
                FT_ENGINE_ERROR("Invalid binary prefab header: {0}", path.string());
                instantiationStack->erase(path);
                return GameObject();
            }

            uint32_t entityCount;
            in.read((char*)&entityCount, sizeof(uint32_t));

            std::map<uint32_t, GameObject> localIdToEntity;
            std::vector<std::pair<GameObject, int32_t>> pendingParents;

            for (uint32_t i = 0; i < entityCount; i++)
            {
                uint32_t localID;
                in.read((char*)&localID, sizeof(uint32_t));
                bool hasRelationship;
                in.read((char*)&hasRelationship, sizeof(bool));
                int32_t parentID;
                in.read((char*)&parentID, sizeof(int32_t));

                std::map<uint32_t, std::vector<char>> componentDataMap;
                std::string prefabPath;

                while (true)
                {
                    uint32_t componentID;
                    in.read((char*)&componentID, sizeof(uint32_t));
                    if (componentID == 0)
                        break;

                    auto* serializer = SerializationSystem::GetSerializerByID(componentID);
                    if (!serializer)
                    {
                        FT_ENGINE_ERROR("Unknown Component ID {0} in Prefab: {1}. Stream may be corrupted.",
                                        componentID,
                                        path.string());
                        while (componentID != 0 && !in.eof())
                            in.read((char*)&componentID, sizeof(uint32_t));
                        break;
                    }

                    std::streampos start = in.tellg();
                    GameObject tempGo;
                    serializer->DeserializeBinary(in, tempGo);
                    std::streampos end = in.tellg();

                    in.seekg(start);
                    std::vector<char> buffer(end - start);
                    in.read(buffer.data(), buffer.size());
                    componentDataMap[componentID] = buffer;

                    if (serializer->Name == "Prefab")
                    {
                        std::stringstream ss(std::string(buffer.begin(), buffer.end()));
                        GameObject pathExtractorGo;
                        pathExtractorGo.AddComponent<Component::Prefab>();
                        serializer->DeserializeBinary(ss, pathExtractorGo);
                        prefabPath = pathExtractorGo.GetComponent<Component::Prefab>().assetPath.string();
                    }
                }

                GameObject go;
                if (!prefabPath.empty() && std::filesystem::exists(prefabPath))
                {
                    go = Instantiate(scene, prefabPath, instantiationStack);
                }
                else
                {
                    go = GameObject(scene->GetRegistry().create(), scene);
                }

                if (!go)
                    continue;
                localIdToEntity[localID] = go;

                for (auto const& [id, data] : componentDataMap)
                {
                    auto* serializer = SerializationSystem::GetSerializerByID(id);
                    if (serializer)
                    {
                        serializer->AddComponent(go);
                        std::stringstream ss(std::string(data.begin(), data.end()));
                        serializer->DeserializeBinary(ss, go);
                    }
                }

                if (hasRelationship)
                    go.AddComponent<Component::Relationship>();
                pendingParents.push_back({ go, parentID });
            }

            for (auto& [go, parentID] : pendingParents)
            {
                if (parentID != -1 && localIdToEntity.count(parentID))
                    go.SetParent(localIdToEntity[parentID]);
                else
                    root = go;
            }

            if (!root && !localIdToEntity.empty())
                root = localIdToEntity.begin()->second;
        }

        instantiationStack->erase(path);
        return root;
    }

} // namespace Frost