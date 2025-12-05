#include "Editor/UI/Scene/SceneView.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Asset/MeshConfig.h"
#include "Frost/Utils/Math/Matrix.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
    using namespace Frost;
    using namespace Frost::Math;

    void SceneView::_HandleMeshDrop(const std::filesystem::path& relativePath)
    {
        if (!_sceneContext)
            return;

        Vector3 spawnPos = _GetSpawnPositionFromMouse();
        auto newEntity = _sceneContext->CreateGameObject(relativePath.stem().string());

        newEntity.AddComponent<Component::Transform>(spawnPos);
        newEntity.AddComponent<Component::StaticMesh>(Component::MeshSourceFile{ relativePath.string() });

        if (_isPrefabView)
        {
            auto view = _sceneContext->GetRegistry().view<Component::Relationship>();
            for (auto entity : view)
            {
                if (view.get<Component::Relationship>(entity).parent == entt::null)
                {
                    newEntity.SetParent(Frost::GameObject(entity, _sceneContext));
                    break;
                }
            }
        }

        _selection = newEntity;
    }

    Frost::Math::Vector3 SceneView::_GetSpawnPositionFromMouse()
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 min = ImGui::GetWindowContentRegionMin();
        ImVec2 max = ImGui::GetWindowContentRegionMax();
        min.x += windowPos.x;
        min.y += windowPos.y;
        max.x += windowPos.x;
        max.y += windowPos.y;

        float viewportW = max.x - min.x;
        float viewportH = max.y - min.y;
        float mx = mousePos.x - min.x;
        float my = mousePos.y - min.y;

        auto [rayOrigin, rayDir] = _GetCameraRay(mx, my, viewportW, viewportH);

        Vector3 planeNormal = { 0.0f, 1.0f, 0.0f };
        Vector3 planePoint = { 0.0f, 0.0f, 0.0f };

        float denominator = Dot(rayDir, planeNormal);
        if (std::abs(denominator) > 0.0001f)
        {
            float t = Dot(planePoint - rayOrigin, planeNormal) / denominator;
            if (t >= 0.0f && t < 1000.0f)
            {
                return rayOrigin + (rayDir * t);
            }
        }

        return rayOrigin + (rayDir * 10.0f);
    }

    std::pair<Frost::Math::Vector3, Frost::Math::Vector3> SceneView::_GetCameraRay(float mouseX,
                                                                                   float mouseY,
                                                                                   float viewportW,
                                                                                   float viewportH)
    {
        using namespace Frost::Math;

        float ndcX = (2.0f * mouseX) / viewportW - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY) / viewportH;

        auto& camTrans = _editorCamera.GetComponent<Component::Transform>();
        auto& camComp = _editorCamera.GetComponent<Component::Camera>();

        Matrix4x4 viewMat = Matrix4x4::Invert(camTrans.GetTransformMatrix());
        float aspectRatio = viewportW / viewportH;
        Matrix4x4 projMat;

        if (camComp.projectionType == Component::Camera::ProjectionType::Perspective)
        {
            projMat = Matrix4x4::CreatePerspectiveFovLH(
                camComp.perspectiveFOV.value(), aspectRatio, camComp.nearClip, camComp.farClip);
        }
        else
        {
            float width = camComp.orthographicSize * aspectRatio;
            projMat =
                Matrix4x4::CreateOrthographicLH(width, camComp.orthographicSize, camComp.nearClip, camComp.farClip);
        }

        Matrix4x4 invViewProj = Matrix4x4::Invert(viewMat * projMat);
        Vector4 nearPointNDC = { ndcX, ndcY, 0.0f, 1.0f };
        Vector4 farPointNDC = { ndcX, ndcY, 1.0f, 1.0f };

        Vector4 nearPointWorld = TransformVector4(nearPointNDC, invViewProj);
        Vector4 farPointWorld = TransformVector4(farPointNDC, invViewProj);

        nearPointWorld = nearPointWorld * (1.0f / nearPointWorld.w);
        farPointWorld = farPointWorld * (1.0f / farPointWorld.w);

        Vector3 rayOrigin = { nearPointWorld.x, nearPointWorld.y, nearPointWorld.z };
        Vector3 rayEnd = { farPointWorld.x, farPointWorld.y, farPointWorld.z };

        return { rayOrigin, Normalize(rayEnd - rayOrigin) };
    }
} // namespace Editor