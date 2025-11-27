#include "Application.h"

#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Event/Event.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Input/Input.h"

#include <cassert>
#include <iostream>

namespace Frost
{
    Application* Application::_singleton = nullptr;

    Application::Application(const ApplicationEntryPoint& entryPoint) :
        _hInstance{ entryPoint.hInstance }, _running{ true }
    {
        FT_ENGINE_ASSERT(!_singleton, "Application already exists!");
        _singleton = this;

#ifdef FT_PLATFORM_WINDOWS
        _window = std::make_unique<WindowWin>(entryPoint.title);
        _renderer = std::make_unique<RendererDX11>();

        RendererAPI::SetRenderer(_renderer.get());
#else
#error "Platform not supported!"
#endif

        _closeEventHandlerId = EventManager::Subscribe<WindowCloseEvent>(FROST_BIND_EVENT_FN(_OnWindowClose));
    }

    void Application::Setup()
    {
        Physics::InitPhysics(_physicsConfig, _physicsConfigured);
    }

    Application::~Application()
    {
        FT_ENGINE_INFO("Application is shutting down...");
        EventManager::Unsubscribe<WindowCloseEvent>(_closeEventHandlerId);

        // Clean up layers
        _layerStack.Clear();

        // Clean up assets
        AssetManager::Shutdown();

        // Clean up physics
        Physics::Shutdown();

        // Clean up renderer
        RendererAPI::SetRenderer(nullptr);
        _renderer.reset();
    }

    void Application::PopLayer(Layer* layer)
    {
        _layerStack.PopLayer(layer);
    }

    void Application::Reset()
    {
        _layerStack.Clear();
    }

    std::optional<Layer*> Application::GetLayer(const Layer::LayerName& name)
    {
        for (const auto& uniqueLayer : _layerStack)
        {
            Layer* layerPtr = uniqueLayer.get();

            if (layerPtr->GetName() == name)
            {
                return layerPtr;
            }
        }

        return std::nullopt;
    }

    void Application::ConfigurePhysics(const PhysicsConfig& config)
    {
        _physicsConfig = config;
        _physicsConfigured = true;
    }

    void Application::Run()
    {
        _renderTimer.Start();
        _physicsTimer.Start();

        while (_running)
        {
            Input::Update();
            EventManager::ProcessEvents();

            if (!_running)
            {
                break;
            }

            Timer::Duration _physicsDuration = _physicsTimer.GetDuration();
            if (_physicsDuration >= _physicsRefreshDuration)
            {
                _physicsTimer.Start();
                _window->MainLoop();

                for (const auto& layer : _layerStack)
                {
                    float fixedDeltaTime =
                        std::chrono::duration<float, std::chrono::seconds::period>(_physicsDuration).count();

                    if (!layer->isPaused())
                    {
                        layer->OnPreFixedUpdate(fixedDeltaTime);
                        layer->OnFixedUpdate(fixedDeltaTime);
                    }
                }
            }

            Timer::Duration _renderDuration = _renderTimer.GetDuration();
            if (_renderDuration >= _renderRefreshDuration)
            {
                _renderTimer.Start();
                RendererAPI::BeginFrame();

                for (const auto& layer : _layerStack)
                {
                    float deltaTime =
                        std::chrono::duration<float, std::chrono::seconds::period>(_renderDuration).count();

                    layer->OnUpdate(deltaTime);
                    layer->OnLateUpdate(deltaTime);
                }

                RendererAPI::EndFrame();
            }
        }
    }

    Application& Application::Get()
    {
        assert(_singleton);
        return *_singleton;
    }

    bool Application::_OnWindowClose(WindowCloseEvent& e)
    {
        _running = false;
        return true;
    }
} // namespace Frost