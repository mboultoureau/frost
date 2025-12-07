#pragma once

#include "Frost/Scene/ECS/GameObject.h"
#include <yaml-cpp/yaml.h>
#include <functional>
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <unordered_map>
#include <typeindex>

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
        std::function<void(GameObject, GameObject)> CopyComponent;

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

            serializer.CopyComponent = [yamlSer, yamlDeser](GameObject source, GameObject destination)
            {
                if (!source || !destination)
                    return;

                YAML::Emitter out;
                out << YAML::BeginMap;
                yamlSer(out, source);
                out << YAML::EndMap;

                if (!destination.HasComponent<T>())
                {
                    destination.AddComponent<T>();
                }

                YAML::Node data = YAML::Load(out.c_str());
                if (data)
                {
                    yamlDeser(data, destination);
                }
            };

            serializer.SerializeYaml = yamlSer;
            serializer.DeserializeYaml = yamlDeser;
            serializer.SerializeBinary = binSer;
            serializer.DeserializeBinary = binDeser;

            GetSerializers().push_back(serializer);
            GetIdMap()[serializer.ID] = &GetSerializers().back();

            GetTypeIdMap()[std::type_index(typeid(T))] = serializer.ID;
        }

        static const std::list<ComponentSerializer>& GetAllSerializers() { return GetSerializers(); }

        static ComponentSerializer* GetSerializerByID(uint32_t id)
        {
            auto it = GetIdMap().find(id);
            if (it != GetIdMap().end())
            {
                return it->second;
            }
            return nullptr;
        }

        template<typename T>
        static ComponentSerializer* GetSerializer()
        {
            auto it = GetTypeIdMap().find(std::type_index(typeid(T)));
            if (it != GetTypeIdMap().end())
            {
                return GetSerializerByID(it->second);
            }
            return nullptr;
        }

    private:
        static std::list<ComponentSerializer>& GetSerializers()
        {
            static std::list<ComponentSerializer> serializers;
            return serializers;
        }

        static std::unordered_map<uint32_t, ComponentSerializer*>& GetIdMap()
        {
            static std::unordered_map<uint32_t, ComponentSerializer*> map;
            return map;
        }

        static std::unordered_map<std::type_index, uint32_t>& GetTypeIdMap()
        {
            static std::unordered_map<std::type_index, uint32_t> map;
            return map;
        }
    };
} // namespace Frost