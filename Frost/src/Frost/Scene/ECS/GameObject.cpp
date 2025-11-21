#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Disabled.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Debugging/Logger.h"

namespace Frost
{
	GameObject::GameObject(entt::entity handle, Scene* scene) : _entityHandle(handle), _scene(scene)
	{
		if (scene)
		{
			_registry = &scene->GetRegistry();
		}
	}

    const GameObject GameObject::InvalidId = GameObject();

    static void DetachFromParent(entt::registry& registry, entt::entity entity)
    {
        auto& childRel = registry.get<Component::Relationship>(entity);
        if (childRel.parent == entt::null) return;

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
            if (current == potentialAncestor) return true;

            if (!registry.all_of<Component::Relationship>(current)) return false;
            current = registry.get<Component::Relationship>(current).parent;
        }
        return false;
    }

    bool GameObject::IsActive() const
    {
        return !_registry->all_of<Component::Disabled>(_entityHandle);
    }

    void GameObject::SetActive(bool active)
    {
        if (active)
        {
            _registry->remove<Component::Disabled>(_entityHandle);
        }
        else
        {
            _registry->emplace_or_replace<Component::Disabled>(_entityHandle);
        }
    }

    void GameObject::SetParent(GameObject parent)
    {
        if (_entityHandle == entt::null) return;
        if (parent && _entityHandle == parent.GetHandle()) return;

        entt::entity parentHandle = parent ? parent.GetHandle() : entt::null;

        if (parentHandle != entt::null)
        {
            if (IsAncestor(*_registry, parentHandle, _entityHandle))
            {
                FT_ENGINE_CRITICAL("Cannot set parent: would create a cycle in the hierarchy.");
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

        if (parentHandle == entt::null) return GameObject();

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
}