#include "Frost/Scene/Serializers/SerializationSystem.h"

namespace Frost
{
    const std::list<ComponentSerializer>& SerializationSystem::GetAllSerializers()
    {
        return GetSerializers();
    }

    ComponentSerializer* SerializationSystem::GetSerializerByID(uint32_t id)
    {
        auto& map = GetIdMap();
        auto it = map.find(id);
        if (it != map.end())
            return it->second;
        return nullptr;
    }

    ComponentSerializer* SerializationSystem::GetSerializerByName(const std::string& name)
    {
        auto& map = GetNameMap();
        auto it = map.find(name);
        if (it != map.end())
            return it->second;
        return nullptr;
    }

    std::list<ComponentSerializer>& SerializationSystem::GetSerializers()
    {
        static std::list<ComponentSerializer> serializers;
        return serializers;
    }

    std::unordered_map<uint32_t, ComponentSerializer*>& SerializationSystem::GetIdMap()
    {
        static std::unordered_map<uint32_t, ComponentSerializer*> map;
        return map;
    }

    std::unordered_map<std::string, ComponentSerializer*>& SerializationSystem::GetNameMap()
    {
        static std::unordered_map<std::string, ComponentSerializer*> map;
        return map;
    }

    std::unordered_map<std::type_index, uint32_t>& SerializationSystem::GetTypeIdMap()
    {
        static std::unordered_map<std::type_index, uint32_t> map;
        return map;
    }
} // namespace Frost