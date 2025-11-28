#pragma once

#include "Frost/Scene/Components/Scriptable.h"
#include <entt/entt.hpp>

namespace Frost
{
    class Scene;
    namespace Component
    {
        struct Scriptable;
    }

    class GameObject
    {
    public:
        using Id = entt::entity;

        GameObject() = default;
        GameObject(entt::entity handle, Scene* scene);

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            return _registry->emplace_or_replace<T>(_entityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent()
        {
            return _registry->get<T>(_entityHandle);
        }

        template<typename T>
        const T& GetComponent() const
        {
            return _registry->get<T>(_entityHandle);
        }

        template<typename T>
        const bool HasComponent() const
        {
            return _registry->all_of<T>(_entityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            _registry->remove<T>(_entityHandle);
        }

        // Activation
        void SetActive(bool active);
        bool IsActive() const;

        // Hierarchy
        void SetParent(GameObject parent);
        GameObject GetParent();
        std::vector<GameObject> GetChildren();

        operator bool() const { return _entityHandle != entt::null; }
        operator entt::entity() const { return _entityHandle; }

        bool operator==(const GameObject& other) const
        {
            return _entityHandle == other._entityHandle && _scene == other._scene;
        }

        GameObject::Id GetId() const { return _entityHandle; }
        entt::entity GetHandle() const { return _entityHandle; }
        Scene* GetScene() const { return _scene; }

        static const GameObject InvalidId;

        template<typename T, typename... Args>
        T& AddScript(Args&&... args);

    private:
        entt::entity _entityHandle{ entt::null };
        Scene* _scene{ nullptr };
        entt::registry* _registry{ nullptr };
    };

    // Implementation
    template<typename T, typename... Args>
    inline T& GameObject::AddScript(Args&&... args)
    {
        if (!HasComponent<Component::Scriptable>())
            AddComponent<Component::Scriptable>();

        auto& scriptable = GetComponent<Component::Scriptable>();
        auto script = std::make_unique<T>(std::forward<Args>(args)...);
        T& scriptRef = *script;
        script->Initialize(*this);
        scriptable._scripts.push_back(std::move(script));
        return scriptRef;
    }
} // namespace Frost
