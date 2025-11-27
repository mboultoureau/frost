#pragma once

#include "Frost/Core/LayerStack.h"
#include "Frost/Core/Timer.h"
#include "Frost/Core/Window.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"
#include "Frost/Physics/PhysicsConfig.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Utils/NoCopy.h"

#include <memory>
#include <optional>
#include <windows.h>

using namespace std::chrono_literals;

namespace Frost
{
    struct ApplicationEntryPoint
    {
        HINSTANCE hInstance;
        HINSTANCE hPrevInstance;
        PWSTR pCmdLine;
        int nCmdShow;

        Window::WindowTitle title;
    };

    class Application : NoCopy
    {
    public:
        Application(const ApplicationEntryPoint& entryPoint);
        virtual ~Application();

        void ConfigurePhysics(const PhysicsConfig& config);
        void Setup();
        void Run();

        // Layers
        template<typename T, typename... Args>
        T* PushLayer(Args&&... args)
        {
            return _layerStack.PushLayer<T>(std::forward<Args>(args)...);
        }

        void PopLayer(Layer* layer);
        std::optional<Layer*> GetLayer(const Layer::LayerName& name);

        void Reset();
        virtual void OnApplicationReady() {};

        static Application& Get();

        LayerStack& GetLayerStack() { return _layerStack; }
        const LayerStack& GetLayerStack() const { return _layerStack; }

        static Window* GetWindow() { return Get()._window.get(); }

    private:
        std::unique_ptr<Window> _window;
        std::unique_ptr<Renderer> _renderer;
        EventHandlerId _closeEventHandlerId;
        bool _running;

    private:
        bool _OnWindowClose(WindowCloseEvent& e);

        HINSTANCE _hInstance;

        LayerStack _layerStack;
        PhysicsConfig _physicsConfig;
        bool _physicsConfigured = false;

        Timer _renderTimer;
        Timer _physicsTimer;

        Timer::Duration _renderRefreshDuration = 16ms;
        Timer::Duration _physicsRefreshDuration = 16ms;

        static Application* _singleton;

        friend class Window;
        friend class Device;
    };

    Application* CreateApplication(ApplicationEntryPoint entryPoint);
} // namespace Frost
