#pragma once

#include "Frost/Scene/ECS/GameObject.h"

#include <vector>
#include <unordered_map>

namespace Frost
{
    class IComponentArray
    {
    public:
        virtual ~IComponentArray() = default;
        virtual void RemoveComponent(GameObject::Id id) = 0;
    };

    template <typename T>
    class ComponentArray final : public IComponentArray
    {
    public:
        void RemoveComponent(GameObject::Id id) override
        {
            if (entityToIndex.count(id) == 0) return;

            size_t indexToRemove = entityToIndex.at(id);
            size_t lastIndex = data.size() - 1;

            if (indexToRemove != lastIndex)
            {
                GameObject::Id entityOfLast = indexToEntity[lastIndex];

                data[indexToRemove] = std::move(data[lastIndex]);

                entityToIndex[entityOfLast] = indexToRemove;
                indexToEntity[indexToRemove] = entityOfLast;
            }

            data.pop_back();
            indexToEntity.pop_back();
            entityToIndex.erase(id);
        }

        std::vector<T> data;
        std::unordered_map<GameObject::Id, size_t> entityToIndex;
        std::vector<GameObject::Id> indexToEntity;
    };
}
