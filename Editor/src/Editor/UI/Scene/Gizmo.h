#pragma once

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Vector.h"
#include "Editor/UI/Scene/SceneViewToolbar.h"
#include "Frost/Scene/Scene.h"

#include <imgui.h>
#include <functional>
#include <optional>

namespace Editor
{
    class Gizmo
    {
    public:
        Gizmo(Frost::Scene* scene);

        void Update(GizmoOperation operation,
                    const std::vector<Frost::GameObject>& targets,
                    const Frost::Math::Matrix4x4& viewMatrix,
                    const Frost::Math::Matrix4x4& projectionMatrix,
                    const Frost::Math::Vector3& rayOrigin,
                    const Frost::Math::Vector3& rayDir,
                    const Frost::Math::Vector3& cameraPosition,
                    const Frost::Math::Vector3& cameraForward,
                    const ImVec2& viewportPos,
                    const ImVec2& viewportSize);

        bool IsManipulating() const { return _isManipulating; }

    private:
        enum class Axis
        {
            None,
            X,
            Y,
            Z,
            XY,
            XZ,
            YZ,
            Uniform,
            Screen
        };

        void _DrawTranslate(const Frost::Math::Matrix4x4& viewProjection,
                            const ImVec2& viewportPos,
                            const ImVec2& viewportSize);
        void _DrawRotate(const Frost::Math::Matrix4x4& viewProjection,
                         const ImVec2& viewportPos,
                         const ImVec2& viewportSize);
        void _DrawScale(const Frost::Math::Matrix4x4& viewProjection,
                        const ImVec2& viewportPos,
                        const ImVec2& viewportSize);

        void _HandlePicking(const Frost::Math::Vector3& rayOrigin, const Frost::Math::Vector3& rayDir);
        void _HandleTransformation(const Frost::Math::Vector3& rayOrigin, const Frost::Math::Vector3& rayDir);

        // Helpers
        ImVec2 _WorldToScreen(const Frost::Math::Vector3& worldPos,
                              const Frost::Math::Matrix4x4& viewProjection,
                              const ImVec2& viewportPos,
                              const ImVec2& viewportSize);

        float _GetGizmoScale(const Frost::Math::Vector3& cameraPos, const Frost::Math::Vector3& cameraForward);

    private:
        // Gizmo
        GizmoOperation _currentOperation = GizmoOperation::None;
        Axis _hoveredAxis = Axis::None;
        Axis _activeAxis = Axis::None;
        bool _isManipulating = false;

        // Transforms
        std::vector<Frost::GameObject> _targets;
        Frost::Math::Vector3 _gizmoPosition;
        Frost::Math::Vector3 _pivotStartPos;
        float _gizmoScreenScale = 1.0f;

        // Manipulation
        Frost::Math::Vector3 _manipulationStartPos;
        std::vector<Frost::Component::Transform> _originalTransforms;

        // Duplicate
        bool _hasDuplicated = false;

        // Context
        Frost::Scene* _scene = nullptr;
    };
} // namespace Editor