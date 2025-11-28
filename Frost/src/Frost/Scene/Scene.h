#pragma once

#include "Frost/Core/Timer.h"
#include "Frost/Scene/Components/Disabled.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Utils/NoCopy.h"
#include "Frost/Asset/Texture.h"

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Frost
{
    class System;
    class GameObject;

    class Scene : NoCopy
    {
    public:
        Scene(std::string&& name = "Scene");
        ~Scene();

        GameObject CreateGameObject(std::string name = "Entity");
        GameObject CreateGameObject(std::string name, GameObject parent);
        void DestroyGameObject(GameObject gameObject);

        void Update(float deltaTime);
        void PreFixedUpdate(float deltaTime);
        void FixedUpdate(float deltaTime);
        void LateUpdate(float deltaTime);
        void SetEditorRenderTarget(std::shared_ptr<Texture> target);

        entt::registry& GetRegistry() { return _registry; }
        const std::string& GetName() const { return _name; }

        template<typename... Components>
        auto View()
        {
            return _registry.view<Components...>();
        }

        template<typename... Components>
        auto ViewActive()
        {
            return _registry.view<Components...>(entt::exclude<Component::Disabled>);
        }

        // Helper methods for backward compatibility and ease of use
        template<typename T, typename... Args>
        T& AddComponent(GameObject gameObject, Args&&... args)
        {
            return gameObject.AddComponent<T>(std::forward<Args>(args)...);
        }

        template<typename T>
        T* GetComponent(GameObject gameObject)
        {
            if (gameObject.HasComponent<T>())
                return &gameObject.GetComponent<T>();
            return nullptr;
        }

        template<typename T, typename... Args>
        T& AddScript(GameObject gameObject, Args&&... args)
        {
            return gameObject.AddScript<T>(std::forward<Args>(args)...);
        }

        GameObject GetGameObjectFromId(GameObject::Id id)
        {
            if (!_registry.valid(id))
            {
                return GameObject();
            }

            return GameObject(id, this);
        }

    private:
        entt::registry _registry;
        std::string _name;
        std::vector<std::unique_ptr<System>> _systems;

        void _InitializeSystems();

        void OnRelationshipDestroyed(entt::registry& registry, entt::entity entity);
    };
} // namespace Frost
