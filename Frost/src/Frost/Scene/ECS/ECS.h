#pragma once

#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/ECS/ComponentArray.h"
#include "Frost/Scene/Components/Script.h"
#include "Frost/Scene/Components/Scriptable.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <typeindex>

namespace Frost
{
    class ECS
    {
    public:
        ~ECS();

        GameObject::Id CreateGameObject();
        void DestroyGameObject(GameObject::Id id);

        void AddScript(GameObject::Id id, std::unique_ptr<Script> script);

        template <typename T, typename... Args>
        void AddComponent(GameObject::Id id, Args&&... args);

        template <typename T>
        void RemoveComponent(GameObject::Id id);

        template <typename T>
        T* GetComponent(const GameObject::Id id);

        template <typename T>
        bool HasComponent(GameObject::Id id);

        template <typename T>
        const std::vector<T>& GetDataArray() const;

        template <typename T>
        const std::vector<GameObject::Id>& GetIndexMap() const;

        template <typename T>
        static std::shared_ptr<ComponentArray<T>>& GetStaticComponentArrayRef()
        {
            static std::shared_ptr<ComponentArray<T>> array = std::make_shared<ComponentArray<T>>();
            return array;
        }

        template <typename T>
        ComponentArray<T>* GetComponents()
        {
            return GetStaticComponentArrayRef<T>().get();
        }

        // Get active game objects
        std::vector<GameObject::Id>& GetActiveGameObjects()
        {
            return _activeGameObjects;
        }

    private:
        GameObject::Id _nextId = GameObject::InvalidId + 1;
        std::vector<GameObject::Id> _activeGameObjects;
        std::unordered_map<size_t, std::shared_ptr<IComponentArray>> _allComponentArrays;
    };
}

namespace Frost
{
    template <typename T, typename... Args>
    void ECS::AddComponent(GameObject::Id id, Args&&... args)
    {
        ComponentArray<T>* array = GetComponents<T>();

        if (array->entityToIndex.count(id)) return;

        size_t typeId = std::type_index(typeid(T)).hash_code();
        if (_allComponentArrays.count(typeId) == 0)
        {
            _allComponentArrays[typeId] = GetStaticComponentArrayRef<T>();
        }

        array->data.emplace_back(std::forward<Args>(args)...);
        
        size_t newIndex = array->data.size() - 1;
        array->entityToIndex[id] = newIndex;
        array->indexToEntity.push_back(id);
    }

    template <typename T>
    void ECS::RemoveComponent(GameObject::Id id)
    {
        GetComponents<T>()->RemoveComponent(id);
    }

    template <typename T>
    T* ECS::GetComponent(const GameObject::Id id)
    {
        ComponentArray<T>* array = GetComponents<T>();
        if (array->entityToIndex.count(id))
        {
            size_t index = array->entityToIndex.at(id);
            return &array->data[index];
        }
        return nullptr;
    }

    template <typename T>
    bool ECS::HasComponent(GameObject::Id id)
    {
        ComponentArray<T>* array = GetComponents<T>();
        return array->entityToIndex.count(id) > 0;
    }

    template <typename T>
    const std::vector<T>& ECS::GetDataArray() const
    {
        return const_cast<ECS*>(this)->GetComponents<T>()->data;
    }

    template <typename T>
    const std::vector<GameObject::Id>& ECS::GetIndexMap() const
    {
        return const_cast<ECS*>(this)->GetComponents<T>()->indexToEntity;
    }
}