#include "Frost/Scene/ECS/ECS.h"

#include <algorithm>

namespace Frost
{
    ECS::~ECS()
    {
        for (auto id : _activeGameObjects)
        {
            for (auto const& [typeId, array] : _allComponentArrays)
            {
                array->RemoveComponent(id);
            }
        }

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

    void ECS::AddScript(GameObject::Id id, std::unique_ptr<Script> script)
    {
        if (!HasComponent<Scriptable>(id))
        {
            AddComponent<Scriptable>(id);
        }

        GetComponent<Scriptable>(id)->_scripts.push_back(std::move(script));
    }
}