#include "Editor/UI/Scene/SceneView.h"

#include "Frost/Debugging/Logger.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Scene/SceneSerializer.h"
#include "Editor/Utils/FileDialogs.h"

#include <imgui.h>

namespace Editor
{
    using namespace Frost;

    void SceneView::_FocusCameraOnEntity(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds)
    {
        using namespace Frost::Math;

        Vector3 center = (bounds.min + bounds.max) * 0.5f;
        float radius =
            std::max({ bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y, bounds.max.z - bounds.min.z }) * 0.5f;
        radius = std::max(radius, 0.1f);

        auto& cameraComp = _editorCamera.GetComponent<Component::Camera>();
        float fovRad = cameraComp.projectionType == Component::Camera::ProjectionType::Perspective
                           ? cameraComp.perspectiveFOV.value()
                           : Angle<Radian>(45.0_deg).value();

        float distance = (radius / std::sin(fovRad * 0.5f)) * 1.1f;
        Vector3 viewDir = Normalize(Vector3{ 0.5f, 0.5f, -1.0f });

        cameraTransform.position = center - (viewDir * distance);
        cameraTransform.LookAt(center);
    }

    void SceneView::_ReparentEntity(entt::entity entityID, entt::entity newParentID)
    {
        Frost::GameObject obj(entityID, _sceneContext);
        Frost::GameObject parentObj =
            (newParentID != entt::null) ? Frost::GameObject(newParentID, _sceneContext) : Frost::GameObject::InvalidId;
        obj.SetParent(parentObj);
    }

    void SceneView::_SavePrefab()
    {
        if (!_isPrefabView || _isReadOnly)
            return;

        auto view = _sceneContext->GetRegistry().view<Component::Relationship>();
        int rootCount = 0;
        entt::entity rootEntity = entt::null;

        for (auto entity : view)
        {
            if (view.get<Component::Relationship>(entity).parent != entt::null)
                continue;
            if (auto* meta = _sceneContext->GetRegistry().try_get<Component::Meta>(entity))
            {
                if (meta->name.find("__EDITOR__") == 0)
                    continue;
            }
            rootCount++;
            rootEntity = entity;
        }

        if (rootCount == 1)
        {
            Frost::PrefabSerializer::CreatePrefab(Frost::GameObject(rootEntity, _sceneContext), _assetPath);
            FT_ENGINE_INFO("Prefab saved successfully: {}", _assetPath.string());
        }
        else
        {
            FT_ENGINE_ERROR("Cannot save Prefab: It must have exactly ONE root entity. Found: {}", rootCount);
            ImGui::OpenPopup("SaveErrorPopup");
        }
    }

    void SceneView::_SaveScene()
    {
        if (_isPrefabView || !_sceneContext)
            return;

        if (_assetPath.empty())
        {
            auto filepath = FileDialogs::SaveFile(
                "Frost Scene (*.scene)\0*.scene\0YAML File (*.yaml)\0*.yaml\0Binary File (*.bin)\0*.bin\0\0");

            if (!filepath)
                return;

            _assetPath = *filepath;
        }

        SceneSerializer serializer(_sceneContext);
        if (serializer.Serialize(_assetPath))
        {
            FT_ENGINE_INFO("Scene saved to: {}", _assetPath.string());
            _title = "Scene: " + _assetPath.stem().string();

            // Also serialize to binary
            std::filesystem::path binaryPath = _assetPath;
            binaryPath.replace_extension(".bin");
            SceneSerializer binarySerializer(_sceneContext);
            if (binarySerializer.Serialize(binaryPath))
            {
                FT_ENGINE_INFO("Scene also saved to binary: {}", binaryPath.string());
            }
            else
            {
                FT_ENGINE_ERROR("Failed to save scene to binary: {}", binaryPath.string());
            }
        }
        else
        {
            FT_ENGINE_ERROR("Failed to save scene to: {}", _assetPath.string());
        }
    }

    void SceneView::_LoadScene()
    {
        if (_isPrefabView || !_sceneContext)
            return;

        auto filepath = FileDialogs::OpenFile("Frost Scene (*.scene;*.yaml;*.bin)\0*.scene;*.yaml;*.bin\0\0");
        if (filepath)
        {
            SceneSerializer serializer(_sceneContext);
            if (serializer.Deserialize(*filepath))
            {
                _selection = {};
                _title = _sceneContext->GetName();
                FT_ENGINE_INFO("Scene loaded from: {}", *filepath);
            }
            else
            {
                FT_ENGINE_ERROR("Failed to load scene from: {}", *filepath);
            }
        }
    }
} // namespace Editor