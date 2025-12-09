#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Core/Layer.h"
#include "Frost/Scene/Scene.h"

#include <memory>

namespace Frost
{
    class FROST_API SceneLayer : public Layer
    {
    public:
        SceneLayer(std::shared_ptr<Scene> scene, const std::string& name = "SceneLayer");

        virtual ~SceneLayer() = default;

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnPreFixedUpdate(float fixedDeltaTime) override;
        virtual void OnFixedUpdate(float fixedDeltaTime) override;
        virtual void OnLateUpdate(float deltaTime) override;

        std::shared_ptr<Scene> GetScene() const;

    private:
        std::shared_ptr<Scene> _scene;
    };
} // namespace Frost