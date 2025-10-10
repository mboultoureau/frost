#include "Frost/Scene/ECS/ECS.h"

#include <algorithm>

namespace Frost
{
    ECS::~ECS()
    {
        _allComponentArrays.clear();
		_activeGameObjects.clear();
    }

    GameObject::Id ECS::CreateGameObject()
    {
        GameObject::Id id = _nextId++;
        _activeGameObjects.push_back(id);
        return id;
    }

    void ECS::DestroyGameObject(GameObject::Id id)
    {
        for (auto const& [typeId, array] : _allComponentArrays) {
            array->RemoveComponent(id);
        }

        _activeGameObjects.erase(
            std::remove(_activeGameObjects.begin(), _activeGameObjects.end(), id),
            _activeGameObjects.end()
        );
    }
}