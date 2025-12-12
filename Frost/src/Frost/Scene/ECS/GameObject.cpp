#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Scene/Components/Disabled.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Scene.h"

namespace Frost
{
    GameObject::GameObject(entt::entity handle) : _entityHandle(handle), _scene(nullptr) {}

    GameObject::GameObject(entt::entity handle, Scene* scene) : _entityHandle(handle), _scene(scene)
    {
        if (scene)
        {
            _registry = &scene->GetRegistry();
        }
    }

    static void DetachFromParent(entt::registry& registry, entt::entity entity)
    {
        auto& childRel = registry.get<Component::Relationship>(entity);
        if (childRel.parent == entt::null)
            return;

        auto& parentRel = registry.get<Component::Relationship>(childRel.parent);
        if (childRel.prevSibling != entt::null)
        {
            auto& prevRel = registry.get<Component::Relationship>(childRel.prevSibling);
            prevRel.nextSibling = childRel.nextSibling;
        }
        else
        {
            parentRel.firstChild = childRel.nextSibling;
        }

        if (childRel.nextSibling != entt::null)
        {
            auto& nextRel = registry.get<Component::Relationship>(childRel.nextSibling);
            nextRel.prevSibling = childRel.prevSibling;
        }

        parentRel.childrenCount--;
        childRel.parent = entt::null;
        childRel.prevSibling = entt::null;
        childRel.nextSibling = entt::null;
    }

    static void AttachToParent(entt::registry& registry, entt::entity child, entt::entity parent)
    {
        auto& parentRel = registry.get_or_emplace<Component::Relationship>(parent);
        auto& childRel = registry.get_or_emplace<Component::Relationship>(child);

        childRel.parent = parent;

        childRel.nextSibling = parentRel.firstChild;
        childRel.prevSibling = entt::null;

        if (parentRel.firstChild != entt::null)
        {
            auto& oldFirst = registry.get<Component::Relationship>(parentRel.firstChild);
            oldFirst.prevSibling = child;
        }

        parentRel.firstChild = child;
        parentRel.childrenCount++;
    }

    static bool IsAncestor(entt::registry& registry, entt::entity entity, entt::entity potentialAncestor)
    {
        auto current = entity;
        while (current != entt::null)
        {
            if (current == potentialAncestor)
                return true;

            if (!registry.all_of<Component::Relationship>(current))
                return false;
            current = registry.get<Component::Relationship>(current).parent;
        }
        return false;
    }

    void DisableChildrenRecursive(entt::registry& registry, entt::entity parent)
    {
        if (!registry.all_of<Component::Relationship>(parent))
        {
            return;
        }

        auto& relationship = registry.get<Component::Relationship>(parent);
        auto currentChildHandle = relationship.firstChild;

        while (currentChildHandle != entt::null)
        {
            registry.emplace_or_replace<Component::Disabled>(currentChildHandle);
            DisableChildrenRecursive(registry, currentChildHandle);

            if (registry.valid(currentChildHandle) && registry.all_of<Component::Relationship>(currentChildHandle))
            {
                currentChildHandle = registry.get<Component::Relationship>(currentChildHandle).nextSibling;
            }
            else
            {
                currentChildHandle = entt::null;
            }
        }
    }

    bool GameObject::IsActive() const
    {
        return !_registry->all_of<Component::Disabled>(_entityHandle);
    }

    void GameObject::SetActive(bool active)
    {
        if (!IsValid())
        {
            return;
        }

        if (active)
        {
            _registry->remove<Component::Disabled>(_entityHandle);
        }
        else
        {
            _registry->emplace_or_replace<Component::Disabled>(_entityHandle);
            DisableChildrenRecursive(*_registry, _entityHandle);
        }
    }

    void GameObject::SetParent(GameObject parent)
    {
        if (_entityHandle == entt::null)
        {
            return;
        }

        if (parent && _entityHandle == parent.GetHandle())
        {
            return;
        }

        entt::entity parentHandle = parent ? parent.GetHandle() : entt::null;

        if (parentHandle != entt::null)
        {
            if (IsAncestor(*_registry, parentHandle, _entityHandle))
            {
                FT_ENGINE_CRITICAL("Cannot set parent: would create a cycle in "
                                   "the hierarchy.");
                return;
            }
        }

        if (_registry->all_of<Component::Relationship>(_entityHandle))
        {
            DetachFromParent(*_registry, _entityHandle);
        }

        if (parentHandle != entt::null)
        {
            AttachToParent(*_registry, _entityHandle, parentHandle);
        }
    }

    GameObject GameObject::GetParent()
    {
        if (!_registry->all_of<Component::Relationship>(_entityHandle))
        {
            return GameObject();
        }

        entt::entity parentHandle = _registry->get<Component::Relationship>(_entityHandle).parent;

        if (parentHandle == entt::null)
            return GameObject();

        return GameObject(parentHandle, _scene);
    }

    std::vector<GameObject> GameObject::GetChildren()
    {
        std::vector<GameObject> children;

        if (!_registry->all_of<Component::Relationship>(_entityHandle))
        {
            return children;
        }

        const auto& rel = _registry->get<Component::Relationship>(_entityHandle);
        children.reserve(rel.childrenCount);

        auto curr = rel.firstChild;
        while (curr != entt::null)
        {
            children.emplace_back(curr, _scene);
            curr = _registry->get<Component::Relationship>(curr).nextSibling;
        }

        return children;
    }

    void GameObject::DestroyAllChildren()
    {
        if (!_scene)
        {
            FT_ENGINE_WARN("GameObject::DestroyAllChildren called on an invalid object (no scene context).");
            return;
        }

        auto childrenCopy = GetChildren();
        for (auto& child : childrenCopy)
        {
            _scene->DestroyGameObject(child);
        }

        if (HasComponent<Component::Relationship>())
        {
            auto& rel = GetComponent<Component::Relationship>();
            rel.firstChild = entt::null;
            rel.childrenCount = 0;
        }
    }

    const bool GameObject::IsValid() const
    {
        return _scene->GetRegistry().valid(_entityHandle);
    }

    // Find by name
    void GetChildrenRecursiveHelper(entt::registry* registry,
                                    Scene* scene,
                                    entt::entity parentHandle,
                                    const std::string& name,
                                    std::vector<GameObject>& results)
    {
        auto* relation = registry->try_get<Component::Relationship>(parentHandle);
        if (!relation)
            return;

        auto currentEntity = relation->firstChild;
        while (currentEntity != entt::null)
        {
            // Check the current child
            if (auto* meta = registry->try_get<Component::Meta>(currentEntity))
            {
                if (meta->name == name)
                {
                    results.emplace_back(currentEntity, scene);
                }
            }

            // Recursively check the child's children
            GetChildrenRecursiveHelper(registry, scene, currentEntity, name, results);

            // Check the next sibling
            if (auto* childRel = registry->try_get<Component::Relationship>(currentEntity))
            {
                currentEntity = childRel->nextSibling;
            }
            else
            {
                currentEntity = entt::null;
            }
        }
    }

    std::vector<GameObject> GameObject::GetChildrenByName(const std::string& name, bool recursive)
    {
        std::vector<GameObject> results;
        if (!IsValid())
            return results;

        if (recursive)
        {
            // Recursive Version
            GetChildrenRecursiveHelper(_registry, _scene, _entityHandle, name, results);
        }
        else
        {
            // Direct Children Only Version
            auto* relation = _registry->try_get<Component::Relationship>(_entityHandle);
            if (!relation)
                return results;

            auto currentEntity = relation->firstChild;
            while (currentEntity != entt::null)
            {
                if (auto* meta = _registry->try_get<Component::Meta>(currentEntity))
                {
                    if (meta->name == name)
                    {
                        results.emplace_back(currentEntity, _scene);
                    }
                }

                auto* childRel = _registry->try_get<Component::Relationship>(currentEntity);
                currentEntity = childRel ? childRel->nextSibling : entt::null;
            }
        }

        return results;
    }

    entt::entity GetChildRecursiveHelper(entt::registry* registry, entt::entity parentHandle, const std::string& name)
    {
        auto* relation = registry->try_get<Component::Relationship>(parentHandle);
        if (!relation)
            return entt::null;

        auto currentEntity = relation->firstChild;
        while (currentEntity != entt::null)
        {
            if (auto* meta = registry->try_get<Component::Meta>(currentEntity))
            {
                if (meta->name == name)
                {
                    return currentEntity;
                }
            }

            entt::entity foundInDeep = GetChildRecursiveHelper(registry, currentEntity, name);
            if (foundInDeep != entt::null)
            {
                return foundInDeep;
            }

            auto* childRel = registry->try_get<Component::Relationship>(currentEntity);
            currentEntity = childRel ? childRel->nextSibling : entt::null;
        }

        return entt::null;
    }

    GameObject GameObject::GetChildByName(const std::string& name, bool recursive)
    {
        if (!IsValid())
            return {};

        if (recursive)
        {
            entt::entity foundHandle = GetChildRecursiveHelper(_registry, _entityHandle, name);
            if (foundHandle != entt::null)
                return GameObject(foundHandle, _scene);
        }
        else
        {
            auto* relation = _registry->try_get<Component::Relationship>(_entityHandle);
            if (!relation)
                return {};

            auto currentEntity = relation->firstChild;
            while (currentEntity != entt::null)
            {
                if (auto* meta = _registry->try_get<Component::Meta>(currentEntity))
                {
                    if (meta->name == name)
                        return GameObject(currentEntity, _scene);
                }

                auto* childRel = _registry->try_get<Component::Relationship>(currentEntity);
                currentEntity = childRel ? childRel->nextSibling : entt::null;
            }
        }

        return {};
    }
} // namespace Frost