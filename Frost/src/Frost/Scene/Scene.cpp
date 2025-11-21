#include "Frost/Scene/Scene.h"

#include "Frost/Scene/Systems/JoltRendererSystem.h"
#include "Frost/Scene/Systems/ModelRendererSystem.h"
#include "Frost/Scene/Systems/WorldTransformSystem.h"
#include "Frost/Scene/Systems/ScriptableSystem.h"
#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/Systems/UISystem.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Relationship.h"

using namespace Frost::Component;

namespace Frost
{
    Scene::Scene(std::string&& name) : _name{ std::move(name) }
    {
        _registry.on_destroy<Component::Relationship>().connect<&Scene::OnRelationshipDestroyed>(this);
        _InitializeSystems();
    }

    Scene::~Scene()
    {
        for (auto& system : _systems)
        {
            if (system)
            {
                system->OnDetach(*this);
            }
        }
        _systems.clear();
        
        _registry.on_destroy<Component::Relationship>().disconnect<&Scene::OnRelationshipDestroyed>(this);
        _registry.clear();
    }

    GameObject Scene::CreateGameObject(std::string name)
    {
        auto entity = _registry.create();
        
        // Meta, Relationship, Transform and WorldTransform is used
        // by 99% of all gameobjects
        _registry.emplace<Component::Meta>(entity, name);
        _registry.emplace<Component::Relationship>(entity);
        _registry.emplace<Component::Transform>(entity);
        _registry.emplace<Component::WorldTransform>(entity);

        return GameObject(entity, this);
    }

    GameObject Scene::CreateGameObject(std::string name, GameObject parent)
    {
        GameObject gameObject = CreateGameObject(name);
        gameObject.SetParent(parent);
        return gameObject;
    }

    void Scene::DestroyGameObject(GameObject gameObject)
    {
        _registry.destroy(gameObject.GetHandle());
    }

    void Scene::_InitializeSystems()
    {
		_systems.push_back(std::make_unique<WorldTransformSystem>());
        _systems.push_back(std::make_unique<ModelRendererSystem>());
		_systems.push_back(std::make_unique<ScriptableSystem>());
        _systems.push_back(std::make_unique<PhysicSystem>());
        _systems.push_back(std::make_unique<UISystem>());
        
#ifdef FT_DEBUG
        _systems.push_back(std::make_unique<JoltRendererSystem>());
#endif
    }

    void Scene::OnRelationshipDestroyed(entt::registry& registry, entt::entity entity)
    {
        auto& relationship = registry.get<Component::Relationship>(entity);

        if (relationship.parent != entt::null)
        {
            if (registry.valid(relationship.parent))
            {
                auto& parentRel = registry.get<Component::Relationship>(relationship.parent);

                if (parentRel.firstChild == entity)
                {
                    parentRel.firstChild = relationship.nextSibling;
                }

                if (parentRel.childrenCount > 0) parentRel.childrenCount--;
            }
        }

        if (relationship.prevSibling != entt::null)
        {
            if (registry.valid(relationship.prevSibling))
            {
                auto& prevRel = registry.get<Component::Relationship>(relationship.prevSibling);
                prevRel.nextSibling = relationship.nextSibling;
            }
        }

        if (relationship.nextSibling != entt::null)
        {
            if (registry.valid(relationship.nextSibling))
            {
                auto& nextRel = registry.get<Component::Relationship>(relationship.nextSibling);
                nextRel.prevSibling = relationship.prevSibling;
            }
        }

        auto curr = relationship.firstChild;
        while (curr != entt::null)
        {
            if (auto* childRel = registry.try_get<Component::Relationship>(curr))
            {
                auto next = childRel->nextSibling;

                childRel->parent = entt::null;
                childRel->prevSibling = entt::null;
                childRel->nextSibling = entt::null;

                registry.destroy(curr);
                curr = next;
            }
            else
            {
                curr = entt::null;
            }
        }
    }

    void Scene::Update(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            system->Update(*this, deltaTime);
        }
    }

    void Scene::FixedUpdate(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            system->FixedUpdate(*this, deltaTime);
        }
    }

    void Scene::LateUpdate(float deltaTime)
    {
        for (const auto& system : _systems)
        {
            system->LateUpdate(*this, deltaTime);
        }
    }
}

