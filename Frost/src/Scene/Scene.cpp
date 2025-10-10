#include "Scene/Scene.h"

#include "Scene/RendererSystem.h"

namespace Frost
{
    Scene::Scene()
    {
        _InitializeSystems();
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
        _systems.push_back(std::make_unique<RendererSystem>());
    }

    void Scene::Update(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            if (system->GetSystemType() == SystemType::Update)
            {
                system->Update(_ecs, deltaTime);
            }
        }
    }

    void Scene::FixedUpdate(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            if (system->GetSystemType() == SystemType::FixedUpdate)
            {
                system->Update(_ecs, deltaTime);
            }
        }
    }

    void Scene::LateUpdate(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            if (system->GetSystemType() == SystemType::LateUpdate)
            {
                system->Update(_ecs, deltaTime);
            }
        }
    }
}

