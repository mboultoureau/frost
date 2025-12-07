#pragma once

namespace Frost::Scripting
{
    class Application;
    class Scene;

    class GameScript
    {
    public:
        // Lifecycle
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnPreFixedUpdate(float fixedDeltaTime) {}
        virtual void OnFixedUpdate(float fixedDeltaTime) {}
        virtual void OnLateUpdate(float deltaTime) {}

        // Application
        virtual void OnApplicationReady(Application* app) {}
        virtual void OnApplicationQuit() {}

        // Scene
        virtual void OnSceneLoaded(Scene* scene) {}
        virtual void OnSceneUnloaded(Scene* scene) {}
        virtual void OnSceneWillLoad(Scene* scene) {}
        virtual void OnSceneWillUnload(Scene* scene) {}
    };
} // namespace Frost::Scripting