#pragma once

#include "Frost/Core/Core.h"
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

    class FROST_API ComponentUIRegistry
    {
    public:
        using DrawCallback = std::function<void(Scene*, entt::entity, const UIContext&)>;

        template<typename T>
        static void Register(DrawCallback callback)
        {
            RegisterByType(std::type_index(typeid(T)),
                           [callback](Scene* scene, entt::entity e, const UIContext& ctx)
                           {
                               if (scene->GetRegistry().all_of<T>(e))
                               {
                                   callback(scene, e, ctx);
                               }
                           });
        }

        template<typename T>
        static void Draw(Scene* scene, entt::entity e, const UIContext& ctx)
        {
            DrawByType(std::type_index(typeid(T)), scene, e, ctx);
        }

        static void DrawAll(Scene* scene, entt::entity e, const UIContext& ctx);
        static void InitEngineComponents();

    private:
        static void DrawByType(std::type_index type, Scene* scene, entt::entity e, const UIContext& ctx);
        static void RegisterByType(std::type_index type, DrawCallback callback);

        static std::unordered_map<std::type_index, DrawCallback> _drawers;
    };
} // namespace Frost