#include "Frost/Core/SceneLayer.h"

namespace Frost
{
    SceneLayer::SceneLayer(std::shared_ptr<Scene> scene, const std::string& name) : Layer(name), _scene(scene) {}

    void SceneLayer::OnUpdate(float deltaTime)
    {
        if (_scene && !isPaused())
        {
            _scene->Update(deltaTime);
        }
    }

    void SceneLayer::OnPreFixedUpdate(float fixedDeltaTime)
    {
        if (_scene && !isPaused())
        {
            _scene->PreFixedUpdate(fixedDeltaTime);
        }
    }

    void SceneLayer::OnFixedUpdate(float fixedDeltaTime)
    {
        if (_scene && !isPaused())
        {
            _scene->FixedUpdate(fixedDeltaTime);
        }
    }

    void SceneLayer::OnLateUpdate(float deltaTime)
    {
        if (_scene && !isPaused())
        {
            _scene->LateUpdate(deltaTime);
        }
    }

    std::shared_ptr<Scene> SceneLayer::GetScene() const
    {
        return _scene;
    }
} // namespace Frost