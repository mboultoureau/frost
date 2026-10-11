#include "Frost/Core/Application.h"

#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Event/Event.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Input/Input.h"
#include "Frost/Debugging/ComponentUIRegistry.h"
#include "Frost/Scripting/ScriptingEngine.h"
#include "Frost/Scene/Serializers/EngineComponentSerializer.h"

#include <cassert>
#include <iostream>
#include <thread>

namespace Frost
{
    Application* Application::_singleton = nullptr;
    std::filesystem::path Application::_projectDirectory = ".";

    Application::Application(const ApplicationSpecification& entryPoint) : _running{ true }
    {
        FT_ENGINE_ASSERT(!_singleton, "Application already exists!");
        _singleton = this;

#ifdef FT_PLATFORM_WINDOWS
        _window = std::make_unique<WindowWin>(entryPoint);
        _renderer = std::make_unique<RendererDX11>();

        RendererAPI::SetRenderer(_renderer.get());

        FT_ENGINE_INFO("Initializing App...");

#else
#error "Platform not supported!"
#endif
        _closeEventHandlerId = EventManager::Subscribe<WindowCloseEvent>(FROST_BIND_EVENT_FN(_OnWindowClose));

        SetTargetFPS(entryPoint.targetFPS);
        SetPhysicsFPS(entryPoint.physicsFPS > 0 ? entryPoint.physicsFPS : 60);

        if (!entryPoint.scriptPath.empty())
        {
            Scripting::ScriptingEngine::LoadScriptingDLL(entryPoint.scriptPath.string());
        }

        // Register engine component serializers
        EngineComponentSerializer::RegisterEngineComponents();
    }

    void Application::Setup()
    {
        Physics::InitPhysics(_physicsConfig, _physicsConfigured);
        Frost::ComponentUIRegistry::InitEngineComponents();
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

    void Application::SetTargetFPS(uint32_t targetFPS)
    {
        _targetFPS = targetFPS;
        if (_targetFPS > 0)
        {
            _targetFrameDuration = std::chrono::duration_cast<Timer::Duration>(
                std::chrono::duration<double>(1.0 / static_cast<double>(_targetFPS)));
        }
        else
        {
            _targetFrameDuration = Timer::Duration::zero();
        }
    }

    void Application::SetPhysicsFPS(uint32_t physicsFPS)
    {
        FT_ENGINE_ASSERT(physicsFPS > 0, "Physics FPS must be greater than zero!");
        _physicsFPS = physicsFPS;
        _fixedDeltaTime = 1.0f / static_cast<float>(_physicsFPS);
        _fixedStepDuration = std::chrono::duration_cast<Timer::Duration>(
            std::chrono::duration<double>(1.0 / static_cast<double>(_physicsFPS)));
    }

    void Application::Run()
    {
        _renderTimer.Start();
        _physicsTimer.Start();

        Timer::Duration accumulator = Timer::Duration::zero();

        while (_running)
        {
            Timer frameTimer;
            frameTimer.Start();

            _window->MainLoop();

            Input::Update();
            EventManager::ProcessEvents();

            if (!_running)
            {
                break;
            }

            Timer::Duration elapsed = _physicsTimer.GetDuration();
            _physicsTimer.Start();

            if (elapsed > _maxDuration)
            {
                elapsed = _maxDuration;
            }

            accumulator += elapsed;

            uint32_t subSteps = 0;
            while (accumulator >= _fixedStepDuration && subSteps < _maxPhysicsSubSteps)
            {
                for (const auto& layer : _layerStack)
                {
                    if (!layer->isPaused())
                    {
                        layer->OnPreFixedUpdate(_fixedDeltaTime);
                        layer->OnFixedUpdate(_fixedDeltaTime);
                    }
                }
                accumulator -= _fixedStepDuration;
                ++subSteps;
            }

            if (subSteps >= _maxPhysicsSubSteps)
            {
                accumulator = Timer::Duration::zero();
            }

            Timer::Duration renderDuration = _renderTimer.GetDuration();
            _renderTimer.Start();

            AssetManager::Update();
            RendererAPI::BeginFrame();

            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(renderDuration).count();

            constexpr float maxDeltaTime = 0.25f;
            if (deltaTime > maxDeltaTime)
            {
                deltaTime = maxDeltaTime;
            }

            for (const auto& layer : _layerStack)
            {
                layer->OnUpdate(deltaTime);
                layer->OnLateUpdate(deltaTime);
            }

            RendererAPI::EndFrame();
            Input::Reset();

            // Handle frame rate limiter and idle sleeping
            Window* window = Application::GetWindow();
            bool isMinimized = window && (window->GetWidth() == 0 || window->GetHeight() == 0);

            if (isMinimized)
            {
                std::this_thread::sleep_for(16ms);
            }
            else if (_targetFPS > 0)
            {
                Timer::Duration frameElapsed = frameTimer.GetDuration();
                if (frameElapsed < _targetFrameDuration)
                {
                    std::this_thread::sleep_for(_targetFrameDuration - frameElapsed);
                }
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

    void Application::SetProjectDirectory(const std::filesystem::path& path)
    {
        _projectDirectory = path;
    }

    const std::filesystem::path& Application::GetProjectDirectory()
    {
        return _projectDirectory;
    }
} // namespace Frost