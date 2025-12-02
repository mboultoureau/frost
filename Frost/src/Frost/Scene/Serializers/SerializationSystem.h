#pragma once

#include "Frost/Scene/ECS/GameObject.h"
#include <yaml-cpp/yaml.h>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace Frost
{
    using SerializeYamlFn = std::function<void(YAML::Emitter&, GameObject)>;
    using DeserializeYamlFn = std::function<void(const YAML::Node&, GameObject&)>;

    using SerializeBinaryFn = std::function<void(std::ostream&, GameObject)>;
    using DeserializeBinaryFn = std::function<void(std::istream&, GameObject&)>;

    struct ComponentSerializer
    {
        std::string Name;
        uint32_t ID;

        std::function<bool(GameObject)> HasComponent;
        std::function<void(GameObject)> AddComponent;

        SerializeYamlFn SerializeYaml;
        DeserializeYamlFn DeserializeYaml;

        SerializeBinaryFn SerializeBinary;
        DeserializeBinaryFn DeserializeBinary;
    };

    class SerializationSystem
    {
    public:
        template<typename T>
        static void RegisterComponent(const std::string& name,
                                      SerializeYamlFn yamlSer,
                                      DeserializeYamlFn yamlDeser,
                                      SerializeBinaryFn binSer,
                                      DeserializeBinaryFn binDeser)
        {
            ComponentSerializer serializer;
            serializer.Name = name;
            serializer.ID = static_cast<uint32_t>(std::hash<std::string>{}(name));

            serializer.HasComponent = [](GameObject go) { return go.HasComponent<T>(); };
            serializer.AddComponent = [](GameObject go)
            {
                if (!go.HasComponent<T>())
                {
                    go.AddComponent<T>();
                }
            };

            serializer.SerializeYaml = yamlSer;
            serializer.DeserializeYaml = yamlDeser;
            serializer.SerializeBinary = binSer;
            serializer.DeserializeBinary = binDeser;

            GetSerializers().push_back(serializer);
            GetIdMap()[serializer.ID] = serializer;
        }

        static const std::vector<ComponentSerializer>& GetAllSerializers() { return GetSerializers(); }

        static ComponentSerializer* GetSerializerByID(uint32_t id)
        {
            if (GetIdMap().find(id) != GetIdMap().end())
                return &GetIdMap()[id];
            return nullptr;
        }

    private:
        static std::vector<ComponentSerializer>& GetSerializers()
        {
            static std::vector<ComponentSerializer> serializers;
            return serializers;
        }

        static std::unordered_map<uint32_t, ComponentSerializer>& GetIdMap()
        {
            static std::unordered_map<uint32_t, ComponentSerializer> map;
            return map;
        }
    };
} // namespace Frost