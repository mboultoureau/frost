#pragma once

#include "Frost/Scene/Scene.h"

#include <entt/entt.hpp>
#include <functional>
#include <unordered_map>
#include <typeindex>

namespace Frost
{
    struct UIContext
    {
        bool isEditor = false;
        float deltaTime = 0.0f;
    };

    class ComponentUIRegistry
    {
    public:
        using DrawCallback = std::function<void(Scene*, entt::entity, const UIContext&)>;

        template<typename T>
        static void Register(DrawCallback callback)
        {
            _drawers[typeid(T)] = [callback](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                if (scene->GetRegistry().all_of<T>(e))
                {
                    callback(scene, e, ctx);
                }
            };
        }

        template<typename T>
        static void Draw(Scene* scene, entt::entity e, const UIContext& ctx)
        {
            auto it = _drawers.find(std::type_index(typeid(T)));

            if (it != _drawers.end())
            {
                it->second(scene, e, ctx);
            }
        }

        static void DrawAll(Scene* scene, entt::entity e, const UIContext& ctx);

        static void InitEngineComponents();

    private:
        static std::unordered_map<std::type_index, std::function<void(Scene*, entt::entity, const UIContext&)>>
            _drawers;
    };
} // namespace Frost