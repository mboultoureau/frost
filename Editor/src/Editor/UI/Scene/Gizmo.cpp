#include "Editor/UI/Scene/Gizmo.h"
#include "Frost/Input/Input.h"
#include "Frost/Utils/Math/Intersection.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Debugging/Logger.h"

#undef max
#undef min
#include <imgui_internal.h>

namespace Editor
{
    using namespace Frost;
    using namespace Frost::Math;

    constexpr float GIZMO_BASE_SIZE = 0.2f;

    constexpr float GIZMO_AXIS_LENGTH = 1.0f;
    constexpr float GIZMO_AXIS_RADIUS = 0.1f;

    constexpr float GIZMO_PLANE_OFFSET = 0.4f;
    constexpr float GIZMO_PLANE_SIZE = 0.4f;

    constexpr float GIZMO_CONE_RADIUS = 0.1f;
    constexpr float GIZMO_CONE_HEIGHT = 0.2f;

    constexpr float GIZMO_SCALE_CUBE_SIZE = 0.2f;

    constexpr float GIZMO_ROTATION_RADIUS = 1.0f;
    constexpr float GIZMO_ROTATION_THICKNESS = 0.1f;
    constexpr int GIZMO_ROTATION_SEGMENTS = 64;

    Gizmo::Gizmo(Scene* scene) : _scene{ scene } {}

    void Gizmo::Update(GizmoOperation operation,
                       Frost::Component::Transform& targetTransform,
                       Frost::GameObject& targetGameObject,
                       const Frost::Math::Matrix4x4& viewMatrix,
                       const Frost::Math::Matrix4x4& projectionMatrix,
                       const Frost::Math::Vector3& rayOrigin,
                       const Frost::Math::Vector3& rayDir,
                       const Frost::Math::Vector3& cameraPosition,
                       const Frost::Math::Vector3& cameraForward,
                       const ImVec2& viewportPos,
                       const ImVec2& viewportSize)
    {
        _currentOperation = operation;
        _targetTransform = &targetTransform;
        _targetGameObject = &targetGameObject;
        _gizmoPosition = _targetTransform->position;
        _gizmoScreenScale = _GetGizmoScale(cameraPosition, cameraForward);
        _ownedDuplicatedObject.reset();

        Matrix4x4 viewProjection = viewMatrix * projectionMatrix;

        if (!_isManipulating)
        {
            _HandlePicking(rayOrigin, rayDir);
        }
        _HandleTransformation(rayOrigin, rayDir);

        ImGui::SetNextWindowPos(viewportPos);
        ImGui::SetNextWindowSize(viewportSize);
        ImGui::Begin("GizmoOverlay",
                     nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoDocking);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(
            viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y), false);

        switch (_currentOperation)
        {
            case GizmoOperation::Translate:
                _DrawTranslate(viewProjection, viewportPos, viewportSize);
                break;
            case GizmoOperation::Rotate:
                _DrawRotate(viewProjection, viewportPos, viewportSize);
                break;
            case GizmoOperation::Scale:
                _DrawScale(viewProjection, viewportPos, viewportSize);
                break;
            default:
                break;
        }

        drawList->PopClipRect();
        ImGui::End();
    }

    float Gizmo::_GetGizmoScale(const Frost::Math::Vector3& cameraPos, const Frost::Math::Vector3& cameraForward)
    {
        Vector3 camToGizmo = _gizmoPosition - cameraPos;
        float distance = Dot(camToGizmo, cameraForward);
        return distance * GIZMO_BASE_SIZE;
    }

    ImVec2 Gizmo::_WorldToScreen(const Vector3& worldPos,
                                 const Matrix4x4& viewProjection,
                                 const ImVec2& viewportPos,
                                 const ImVec2& viewportSize)
    {
        Vector4 clipPos = TransformVector4({ worldPos.x, worldPos.y, worldPos.z, 1.0f }, viewProjection);

        if (clipPos.w < 0.1f)
            return { -FLT_MAX, -FLT_MAX };

        Vector3 ndc = { clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w };

        float screenX = viewportPos.x + (ndc.x + 1.0f) * 0.5f * viewportSize.x;
        float screenY = viewportPos.y + (1.0f - (ndc.y + 1.0f) * 0.5f) * viewportSize.y;

        return { screenX, screenY };
    }

    void Gizmo::_HandlePicking(const Vector3& rayOrigin, const Vector3& rayDir)
    {
        _hoveredAxis = Axis::None;
        if (_currentOperation == GizmoOperation::None)
            return;

        float closest_t = std::numeric_limits<float>::max();
        Ray ray = { rayOrigin, rayDir };
        float t;

        if (_currentOperation == GizmoOperation::Translate)
        {
            float scaledRadius = GIZMO_AXIS_RADIUS * _gizmoScreenScale;
            float scaledLength = GIZMO_AXIS_LENGTH * _gizmoScreenScale;
            float scaledPlaneOffset = GIZMO_PLANE_OFFSET * _gizmoScreenScale;
            float scaledPlaneSize = GIZMO_PLANE_SIZE * _gizmoScreenScale;

            Vector3 xy_p0 = _gizmoPosition + Vector3{ scaledPlaneOffset, scaledPlaneOffset, 0 };
            Vector3 xy_p1 = _gizmoPosition + Vector3{ scaledPlaneOffset + scaledPlaneSize, scaledPlaneOffset, 0 };
            Vector3 xy_p2 =
                _gizmoPosition + Vector3{ scaledPlaneOffset + scaledPlaneSize, scaledPlaneOffset + scaledPlaneSize, 0 };
            Vector3 xy_p3 = _gizmoPosition + Vector3{ scaledPlaneOffset, scaledPlaneOffset + scaledPlaneSize, 0 };
            if (IntersectRayQuad(ray, xy_p0, xy_p1, xy_p2, xy_p3, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::XY;
            }

            Vector3 xz_p0 = _gizmoPosition + Vector3{ scaledPlaneOffset, 0, scaledPlaneOffset };
            Vector3 xz_p1 = _gizmoPosition + Vector3{ scaledPlaneOffset + scaledPlaneSize, 0, scaledPlaneOffset };
            Vector3 xz_p2 =
                _gizmoPosition + Vector3{ scaledPlaneOffset + scaledPlaneSize, 0, scaledPlaneOffset + scaledPlaneSize };
            Vector3 xz_p3 = _gizmoPosition + Vector3{ scaledPlaneOffset, 0, scaledPlaneOffset + scaledPlaneSize };
            if (IntersectRayQuad(ray, xz_p0, xz_p1, xz_p2, xz_p3, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::XZ;
            }

            Vector3 yz_p0 = _gizmoPosition + Vector3{ 0, scaledPlaneOffset, scaledPlaneOffset };
            Vector3 yz_p1 = _gizmoPosition + Vector3{ 0, scaledPlaneOffset + scaledPlaneSize, scaledPlaneOffset };
            Vector3 yz_p2 =
                _gizmoPosition + Vector3{ 0, scaledPlaneOffset + scaledPlaneSize, scaledPlaneOffset + scaledPlaneSize };
            Vector3 yz_p3 = _gizmoPosition + Vector3{ 0, scaledPlaneOffset, scaledPlaneOffset + scaledPlaneSize };
            if (IntersectRayQuad(ray, yz_p0, yz_p1, yz_p2, yz_p3, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::YZ;
            }

            if (IntersectRayCylinder(ray, _gizmoPosition, { 1, 0, 0 }, scaledRadius, scaledLength, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::X;
            }
            if (IntersectRayCylinder(ray, _gizmoPosition, { 0, 1, 0 }, scaledRadius, scaledLength, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Y;
            }
            if (IntersectRayCylinder(ray, _gizmoPosition, { 0, 0, 1 }, scaledRadius, scaledLength, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Z;
            }
        }
        else if (_currentOperation == GizmoOperation::Rotate)
        {
            float scaledRadius = GIZMO_ROTATION_RADIUS * _gizmoScreenScale;
            float scaledThickness = GIZMO_ROTATION_THICKNESS * _gizmoScreenScale;

            // Rotation Axis Y (vertical)
            if (IntersectRayTorus(ray, _gizmoPosition, { 0, 1, 0 }, scaledRadius, scaledThickness, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Y;
            }
            // Rotation axis X (right)
            if (IntersectRayTorus(ray, _gizmoPosition, { 1, 0, 0 }, scaledRadius, scaledThickness, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::X;
            }
            // Rotation axis Z (forward)
            if (IntersectRayTorus(ray, _gizmoPosition, { 0, 0, 1 }, scaledRadius, scaledThickness, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Z;
            }
        }
        else if (_currentOperation == GizmoOperation::Scale)
        {
            float scaledRadius = GIZMO_AXIS_RADIUS * _gizmoScreenScale;
            float scaledLength = GIZMO_AXIS_LENGTH * _gizmoScreenScale;

            if (IntersectRayCylinder(ray, _gizmoPosition, { 1, 0, 0 }, scaledRadius, scaledLength, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::X;
            }
            if (IntersectRayCylinder(ray, _gizmoPosition, { 0, 1, 0 }, scaledRadius, scaledLength, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Y;
            }
            if (IntersectRayCylinder(ray, _gizmoPosition, { 0, 0, 1 }, scaledRadius, scaledLength, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Z;
            }

            if (IntersectRayCylinder(ray, _gizmoPosition, { 1, 0, 0 }, scaledRadius * 2.0f, 0.1f, t) && t < closest_t)
            {
                closest_t = t;
                _hoveredAxis = Axis::Uniform;
            }
        }
    }

    void Gizmo::_DrawTranslate(const Frost::Math::Matrix4x4& viewProjection,
                               const ImVec2& viewportPos,
                               const ImVec2& viewportSize)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float thickness = 3.0f;
        const float manipThickness = 5.0f;

        Vector3 origin = _gizmoPosition;
        float length = GIZMO_AXIS_LENGTH * _gizmoScreenScale;
        float coneHeight = GIZMO_CONE_HEIGHT * _gizmoScreenScale;
        float coneRadius = GIZMO_CONE_RADIUS * _gizmoScreenScale;

        auto drawAxis = [&](Axis axis, const Vector3& axisDir, ImU32 color)
        {
            bool isHovered = (_hoveredAxis == axis);
            bool isActive = (_activeAxis == axis);
            ImU32 finalColor = isHovered || isActive ? IM_COL32(255, 255, 0, 255) : color;

            Vector3 end = origin + (axisDir * length);
            ImVec2 screenOrigin = _WorldToScreen(origin, viewProjection, viewportPos, viewportSize);
            ImVec2 screenEnd = _WorldToScreen(end, viewProjection, viewportPos, viewportSize);

            if (screenOrigin.x < -1000 || screenEnd.x < -1000)
                return;

            drawList->AddLine(screenOrigin, screenEnd, finalColor, isActive ? manipThickness : thickness);

            // Cone
            Vector3 coneBase = end;
            Vector3 up = { 0, 1, 0 };
            if (abs(Dot(axisDir, up)) > 0.99f)
                up = { 1, 0, 0 };
            Vector3 side1 = Normalize(Cross(axisDir, up)) * coneRadius;
            Vector3 side2 = Normalize(Cross(axisDir, side1)) * coneRadius;

            ImVec2 coneP1 = _WorldToScreen(coneBase + side1, viewProjection, viewportPos, viewportSize);
            ImVec2 coneP2 = _WorldToScreen(coneBase - side1, viewProjection, viewportPos, viewportSize);
            ImVec2 coneP3 = _WorldToScreen(coneBase + side2, viewProjection, viewportPos, viewportSize);
            ImVec2 coneP4 = _WorldToScreen(coneBase - side2, viewProjection, viewportPos, viewportSize);
            ImVec2 coneTop = _WorldToScreen(end + axisDir * coneHeight, viewProjection, viewportPos, viewportSize);

            drawList->AddTriangleFilled(coneP1, coneP2, coneTop, finalColor);
            drawList->AddTriangleFilled(coneP3, coneP4, coneTop, finalColor);
        };

        drawAxis(Axis::X, { 1, 0, 0 }, IM_COL32(255, 50, 50, 255));
        drawAxis(Axis::Y, { 0, 1, 0 }, IM_COL32(50, 255, 50, 255));
        drawAxis(Axis::Z, { 0, 0, 1 }, IM_COL32(50, 50, 255, 255));

        float offset = GIZMO_PLANE_OFFSET * _gizmoScreenScale;
        float size = GIZMO_PLANE_SIZE * _gizmoScreenScale;

        auto drawPlane = [&](Axis axis, const Vector3& p1_offset, const Vector3& p2_offset, ImU32 color)
        {
            bool isHovered = (_hoveredAxis == axis);
            bool isActive = (_activeAxis == axis);
            ImU32 finalColor = isHovered || isActive ? IM_COL32(255, 255, 0, 180) : color;

            Vector3 p0 = origin + p1_offset * offset + p2_offset * offset;
            Vector3 p1 = p0 + p1_offset * size;
            Vector3 p2 = p1 + p2_offset * size;
            Vector3 p3 = p0 + p2_offset * size;

            ImVec2 s0 = _WorldToScreen(p0, viewProjection, viewportPos, viewportSize);
            ImVec2 s1 = _WorldToScreen(p1, viewProjection, viewportPos, viewportSize);
            ImVec2 s2 = _WorldToScreen(p2, viewProjection, viewportPos, viewportSize);
            ImVec2 s3 = _WorldToScreen(p3, viewProjection, viewportPos, viewportSize);

            if (s0.x < -1000)
                return;

            drawList->AddQuadFilled(s0, s1, s2, s3, finalColor);
        };

        drawPlane(Axis::XY, { 1, 0, 0 }, { 0, 1, 0 }, IM_COL32(50, 50, 255, 100));
        drawPlane(Axis::XZ, { 1, 0, 0 }, { 0, 0, 1 }, IM_COL32(50, 255, 50, 100));
        drawPlane(Axis::YZ, { 0, 1, 0 }, { 0, 0, 1 }, IM_COL32(255, 50, 50, 100));
    }

    void Gizmo::_DrawRotate(const Frost::Math::Matrix4x4& viewProjection,
                            const ImVec2& viewportPos,
                            const ImVec2& viewportSize)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float radius = GIZMO_ROTATION_RADIUS * _gizmoScreenScale;

        auto drawCircle = [&](Axis axis, const Vector3& normal, ImU32 color)
        {
            bool isHovered = (_hoveredAxis == axis);
            bool isActive = (_activeAxis == axis);
            ImU32 finalColor = isHovered || isActive ? IM_COL32(255, 255, 0, 255) : color;
            float thickness = isHovered || isActive ? 4.0f : 2.0f;

            Vector3 right;
            if (abs(Dot(normal, { 0, 1, 0 })) > 0.99f)
                right = { 1, 0, 0 };
            else
                right = Normalize(Cross(normal, { 0, 1, 0 }));

            Vector3 up = Normalize(Cross(right, normal));

            std::vector<ImVec2> points;
            points.reserve(GIZMO_ROTATION_SEGMENTS + 1);

            for (int i = 0; i <= GIZMO_ROTATION_SEGMENTS; ++i)
            {
                float angle = (float)i / (float)GIZMO_ROTATION_SEGMENTS * 2.0f * 3.1415926535f;
                Vector3 point3D = _gizmoPosition + (right * cosf(angle) + up * sinf(angle)) * radius;
                ImVec2 p = _WorldToScreen(point3D, viewProjection, viewportPos, viewportSize);
                if (p.x < -1000)
                    return;
                points.push_back(p);
            }
            drawList->AddPolyline(points.data(), points.size(), finalColor, false, thickness);
        };

        // Dessine les 3 cercles
        drawCircle(Axis::Y, { 0, 1, 0 }, IM_COL32(50, 255, 50, 255)); // Green
        drawCircle(Axis::X, { 1, 0, 0 }, IM_COL32(255, 50, 50, 255)); // Red
        drawCircle(Axis::Z, { 0, 0, 1 }, IM_COL32(50, 50, 255, 255)); // Blue
    }

    void Gizmo::_HandleTransformation(const Vector3& rayOrigin, const Vector3& rayDir)
    {
        auto& mouse = Input::GetMouse();

        if (_hoveredAxis != Axis::None && mouse.IsButtonDown(Mouse::MouseBoutton::Left) && !_isManipulating)
        {
            if (ImGui::IsKeyDown(ImGuiKey_LeftAlt))
            {
                GameObject newGameObject = _scene->DuplicateGameObject(*_targetGameObject);
                if (newGameObject)
                {
                    _ownedDuplicatedObject = newGameObject;

                    if (_onTargetDuplicated)
                        _onTargetDuplicated(newGameObject);

                    _targetGameObject = &(*_ownedDuplicatedObject);
                    _targetTransform = &_targetGameObject->GetComponent<Frost::Component::Transform>();
                }
            }

            _isManipulating = true;
            _activeAxis = _hoveredAxis;
            _originalTransform = *_targetTransform;

            Vector3 planeNormal;
            if (_currentOperation == GizmoOperation::Rotate)
            {
                if (_activeAxis == Axis::X)
                    planeNormal = { 1, 0, 0 };
                else if (_activeAxis == Axis::Y)
                    planeNormal = { 0, 1, 0 };
                else
                    planeNormal = { 0, 0, 1 }; // Z
            }
            else // Translate or Scale
            {
                if (_activeAxis == Axis::Uniform)
                    planeNormal = Normalize(_gizmoPosition - rayOrigin);
                else if (_activeAxis >= Axis::X && _activeAxis <= Axis::Z)
                {
                    Vector3 axisDir;
                    if (_activeAxis == Axis::X)
                        axisDir = { 1, 0, 0 };
                    else if (_activeAxis == Axis::Y)
                        axisDir = { 0, 1, 0 };
                    else
                        axisDir = { 0, 0, 1 };
                    planeNormal = Cross(axisDir, rayDir);
                    planeNormal = Normalize(Cross(planeNormal, axisDir));
                }
                else // Plans
                {
                    if (_activeAxis == Axis::XY)
                        planeNormal = { 0, 0, 1 };
                    if (_activeAxis == Axis::XZ)
                        planeNormal = { 0, 1, 0 };
                    if (_activeAxis == Axis::YZ)
                        planeNormal = { 1, 0, 0 };
                }
            }

            float t;
            Ray ray = { rayOrigin, rayDir };
            if (IntersectRayPlane(ray, _gizmoPosition, planeNormal, t))
            {
                _manipulationStartPos = rayOrigin + (rayDir * t);
            }
        }

        if (!mouse.IsButtonDown(Mouse::MouseBoutton::Left))
        {
            _isManipulating = false;
            _activeAxis = Axis::None;
        }

        if (_isManipulating)
        {
            Vector3 planeNormal;
            Vector3 axisDir = { 0, 0, 0 };

            if (_currentOperation == GizmoOperation::Rotate)
            {
                if (_activeAxis == Axis::X)
                    planeNormal = { 1, 0, 0 };
                else if (_activeAxis == Axis::Y)
                    planeNormal = { 0, 1, 0 };
                else
                    planeNormal = { 0, 0, 1 }; // Z
            }
            else
            {
                if (_activeAxis == Axis::Uniform)
                    planeNormal = Normalize(_gizmoPosition - rayOrigin);
                else if (_activeAxis >= Axis::X && _activeAxis <= Axis::Z)
                {
                    if (_activeAxis == Axis::X)
                        axisDir = { 1, 0, 0 };
                    else if (_activeAxis == Axis::Y)
                        axisDir = { 0, 1, 0 };
                    else
                        axisDir = { 0, 0, 1 };
                    planeNormal = Cross(axisDir, rayDir);
                    planeNormal = Normalize(Cross(planeNormal, axisDir));
                }
                else
                {
                    if (_activeAxis == Axis::XY)
                        planeNormal = { 0, 0, 1 };
                    if (_activeAxis == Axis::XZ)
                        planeNormal = { 0, 1, 0 };
                    if (_activeAxis == Axis::YZ)
                        planeNormal = { 1, 0, 0 };
                }
            }

            bool modified = false;
            Ray ray = { rayOrigin, rayDir };
            float t;
            if (IntersectRayPlane(ray, _manipulationStartPos, planeNormal, t))
            {
                Vector3 currentIntersection = rayOrigin + (rayDir * t);

                if (_currentOperation == GizmoOperation::Rotate)
                {
                    Vector3 startVec = Normalize(_manipulationStartPos - _gizmoPosition);
                    Vector3 currentVec = Normalize(currentIntersection - _gizmoPosition);

                    float angle = acosf(std::clamp(Dot(startVec, currentVec), -1.0f, 1.0f));
                    Vector3 cross = Cross(startVec, currentVec);
                    if (Dot(planeNormal, cross) < 0)
                    {
                        angle = -angle;
                    }

                    using namespace DirectX;
                    XMVECTOR originalQuat =
                        XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_originalTransform.rotation));
                    XMVECTOR deltaQuat =
                        XMQuaternionRotationAxis(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&planeNormal)), angle);
                    XMVECTOR newQuat = XMQuaternionMultiply(deltaQuat, originalQuat);

                    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_targetTransform->rotation), newQuat);
                    modified = true;
                }
                else // Translate or Scale
                {
                    Vector3 delta = currentIntersection - _manipulationStartPos;
                    if (_currentOperation == GizmoOperation::Translate)
                    {
                        if (_activeAxis >= Axis::X && _activeAxis <= Axis::Z)
                            _targetTransform->position = _originalTransform.position + (axisDir * Dot(delta, axisDir));
                        else
                            _targetTransform->position = _originalTransform.position + delta;

                        modified = true;
                    }
                    else if (_currentOperation == GizmoOperation::Scale)
                    {
                        if (_activeAxis == Axis::Uniform)
                        {
                            float moveAmount = (delta.x + delta.y + delta.z) * 0.33f;
                            _targetTransform->scale =
                                _originalTransform.scale + Vector3{ moveAmount, moveAmount, moveAmount };
                        }
                        else
                        {
                            _targetTransform->scale = _originalTransform.scale + (axisDir * Dot(delta, axisDir));
                        }
                        _targetTransform->scale.x = std::max(_targetTransform->scale.x, 0.001f);
                        _targetTransform->scale.y = std::max(_targetTransform->scale.y, 0.001f);
                        _targetTransform->scale.z = std::max(_targetTransform->scale.z, 0.001f);

                        modified = true;
                    }
                }

                if (modified && _targetGameObject)
                {
                    if (auto* physicSystem = _scene->GetSystem<PhysicSystem>())
                    {
                        physicSystem->NotifyRigidBodyUpdate(*_scene, *_targetGameObject);
                        FT_ENGINE_TRACE("RigidBody component on entity {} updated in Gizmo.",
                                        (uint64_t)_targetGameObject->GetHandle());
                    }
                }
            }
        }
    }

    void Gizmo::_DrawScale(const Frost::Math::Matrix4x4& viewProjection,
                           const ImVec2& viewportPos,
                           const ImVec2& viewportSize)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float thickness = 3.0f;
        const float manipThickness = 5.0f;

        Vector3 origin = _gizmoPosition;
        float length = GIZMO_AXIS_LENGTH * _gizmoScreenScale;
        float cubeSize = GIZMO_SCALE_CUBE_SIZE * _gizmoScreenScale;

        auto drawAxis = [&](Axis axis, const Vector3& axisDir, ImU32 color)
        {
            bool isHovered = (_hoveredAxis == axis);
            bool isActive = (_activeAxis == axis);
            ImU32 finalColor = isHovered || isActive ? IM_COL32(255, 255, 0, 255) : color;

            Vector3 end = origin + (axisDir * length);
            ImVec2 screenOrigin = _WorldToScreen(origin, viewProjection, viewportPos, viewportSize);
            ImVec2 screenEnd = _WorldToScreen(end, viewProjection, viewportPos, viewportSize);

            if (screenOrigin.x < -1000 || screenEnd.x < -1000)
                return;

            drawList->AddLine(screenOrigin, screenEnd, finalColor, isActive ? manipThickness : thickness);

            Vector3 cubeCenter = end;
            Vector3 up = { 0, 1, 0 };
            if (abs(Dot(axisDir, up)) > 0.99f)
                up = { 1, 0, 0 };
            Vector3 side1 = Normalize(Cross(axisDir, up)) * (cubeSize / 2.0f);
            Vector3 side2 = Normalize(Cross(axisDir, side1)) * (cubeSize / 2.0f);

            Vector3 p0 = cubeCenter - side1 - side2;
            Vector3 p1 = cubeCenter + side1 - side2;
            Vector3 p2 = cubeCenter + side1 + side2;
            Vector3 p3 = cubeCenter - side1 + side2;

            ImVec2 s0 = _WorldToScreen(p0, viewProjection, viewportPos, viewportSize);
            ImVec2 s1 = _WorldToScreen(p1, viewProjection, viewportPos, viewportSize);
            ImVec2 s2 = _WorldToScreen(p2, viewProjection, viewportPos, viewportSize);
            ImVec2 s3 = _WorldToScreen(p3, viewProjection, viewportPos, viewportSize);

            drawList->AddQuadFilled(s0, s1, s2, s3, finalColor);
            drawList->AddQuad(s0, s1, s2, s3, IM_COL32(0, 0, 0, 128), 1.0f);
        };

        drawAxis(Axis::X, { 1, 0, 0 }, IM_COL32(255, 50, 50, 255));
        drawAxis(Axis::Y, { 0, 1, 0 }, IM_COL32(50, 255, 50, 255));
        drawAxis(Axis::Z, { 0, 0, 1 }, IM_COL32(50, 50, 255, 255));

        bool isCenterHovered = (_hoveredAxis == Axis::Uniform);
        bool isCenterActive = (_activeAxis == Axis::Uniform);
        ImU32 centerColor =
            isCenterHovered || isCenterActive ? IM_COL32(255, 255, 0, 255) : IM_COL32(200, 200, 200, 255);

        float centerCubeSize = cubeSize * 0.75f;
        Vector3 c_p0 = origin + Vector3{ -centerCubeSize, -centerCubeSize, 0 };
        Vector3 c_p1 = origin + Vector3{ centerCubeSize, -centerCubeSize, 0 };
        Vector3 c_p2 = origin + Vector3{ centerCubeSize, centerCubeSize, 0 };
        Vector3 c_p3 = origin + Vector3{ -centerCubeSize, centerCubeSize, 0 };

        ImVec2 sOrigin = _WorldToScreen(origin, viewProjection, viewportPos, viewportSize);
        float screenCenterSize = centerCubeSize * 50.0f;

        ImVec2 s0 = { sOrigin.x - screenCenterSize, sOrigin.y - screenCenterSize };
        ImVec2 s1 = { sOrigin.x + screenCenterSize, sOrigin.y - screenCenterSize };
        ImVec2 s2 = { sOrigin.x + screenCenterSize, sOrigin.y + screenCenterSize };
        ImVec2 s3 = { sOrigin.x - screenCenterSize, sOrigin.y + screenCenterSize };

        drawList->AddQuadFilled(s0, s1, s2, s3, centerColor);
        drawList->AddQuad(s0, s1, s2, s3, IM_COL32(0, 0, 0, 128), 1.0f);
    }
} // namespace Editor