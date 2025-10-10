#include "Frost/Scene/Scene.h"

#include "Frost/Scene/Systems/RendererSystem.h"
#include "Frost/Scene/Components/GameObjectInfo.h"

namespace Frost
{
    Scene::Scene()
    {
        _InitializeSystems();
    }

    GameObject::Id Scene::CreateGameObject(std::string name)
    {
        auto id = _ecs.CreateGameObject();
        _ecs.AddComponent<GameObjectInfo>(id, id, name);
        return id;
    }

    GameObject::Id Scene::CreateGameObject()
    {
        return _ecs.CreateGameObject();
    }

    void Scene::DestroyGameObject(GameObject::Id id)
    {
        _ecs.DestroyGameObject(id);
    }

    void Scene::_InitializeSystems()
    {
        _systems.push_back(std::make_unique<Frost::RendererSystem>());
    }

    void Scene::Update(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            system->Update(_ecs, deltaTime);
        }
    }

    void Scene::FixedUpdate(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            system->FixedUpdate(_ecs, deltaTime);
        }
    }

    void Scene::LateUpdate(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            system->LateUpdate(_ecs, deltaTime);
        }
    }
}

