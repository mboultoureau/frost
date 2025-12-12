#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Core/Layer.h"
#include "Frost/Debugging/DebugInterface/DebugPanel.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Utils/NoCopy.h"

#include <memory>
#include <vector>

namespace Frost
{
    class FROST_API DebugLayer : public Layer, NoCopy
    {
    public:
        DebugLayer();
        virtual ~DebugLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnLateUpdate(float deltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        void AddScene(Scene* scene);
        void RemoveScene(Scene* scene);

        static Layer::LayerName GetStaticName() { return "DebugLayer"; }

    private:
        bool _displayDebug = false;
        std::vector<std::unique_ptr<DebugPanel>> _debugPanels;
    };
} // namespace Frost
