#include "Frost/Scene/Scene.h"

#include "Frost/Scene/Systems/JoltRendererSystem.h"
#include "Frost/Scene/Systems/ModelRendererSystem.h"
#include "Frost/Scene/Systems/WorldTransformSystem.h"
#include "Frost/Scene/Systems/ScriptableSystem.h"
#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/Systems/UISystem.h"
#include "Frost/Scene/Components/Meta.h"

using namespace Frost::Component;

namespace Frost
{
    Scene::Scene(std::string&& name) : _name{ std::move(name) }
    {
        _InitializeSystems();
    }

    GameObject::Id Scene::CreateGameObject(std::string name)
    {
        auto id = _ecs.CreateGameObject();
        _ecs.AddComponent<Meta>(id, id, name);
        return id;
    }

    GameObject::Id Scene::CreateGameObject(std::string name, GameObject::Id parentId)
    {
        auto id = _ecs.CreateGameObject();
        _ecs.AddComponent<Meta>(id, id, parentId, name);
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
		_systems.push_back(std::make_unique<WorldTransformSystem>());
        _systems.push_back(std::make_unique<ModelRendererSystem>());
		_systems.push_back(std::make_unique<ScriptableSystem>());
        _systems.push_back(std::make_unique<PhysicSystem>());
        _systems.push_back(std::make_unique<UISystem>());
        //_systems.push_back(std::make_unique<JoltRendererSystem>());
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

