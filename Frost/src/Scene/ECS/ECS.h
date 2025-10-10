#pragma once

#include "Scene/ECS/GameObject.h"
#include "Scene/ECS/ComponentArray.h"

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
        GameObject::Id CreateGameObject();
        void DestroyGameObject(GameObject::Id id);

        template <typename T, typename... Args>
        void AddComponent(GameObject::Id id, Args&&... args);

        template <typename T>
        void RemoveComponent(GameObject::Id id);

        template <typename T>
        T* GetComponent(GameObject::Id id);

        template <typename T>
        const std::vector<T>& GetDataArray() const;

        template <typename T>
        const std::vector<GameObject::Id>& GetIndexMap() const;

        template <typename T>
        ComponentArray<T>* GetComponents()
        {
            static std::shared_ptr<ComponentArray<T>> array = std::make_shared<ComponentArray<T>>();
            return array.get();
        }

    private:
        GameObject::Id _nextId = 0;
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

        if (_allComponentArrays.count(std::type_index(typeid(T)).hash_code()) == 0)
        {
            _allComponentArrays[std::type_index(typeid(T)).hash_code()] = GetComponentArrayStaticStorage<T>();
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
    T* ECS::GetComponent(GameObject::Id id)
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
    const std::vector<T>& ECS::GetDataArray() const
    {
        return const_cast<ECS*>(this)->GetComponents<T>()->Data;
    }

    template <typename T>
    const std::vector<GameObject::Id>& ECS::GetIndexMap() const
    {
        return const_cast<ECS*>(this)->GetComponents<T>()->IndexToEntity;
    }
}