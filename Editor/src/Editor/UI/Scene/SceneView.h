#pragma once

#include "Editor/UI/EditorPanel.h"
#include "Editor/Utils/EditorCameraController.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Renderer/BoundingBox.h"
#include "Editor/UI/Scene/SceneViewToolbar.h"
#include "Editor/UI/Scene/Gizmo.h"

#include <imgui.h>
#include <string>
#include <memory>
#include <filesystem>

namespace Editor
{
    class SceneView : public EditorPanel
    {
    public:
        struct MeshPreviewTag
        {
        };

        struct SceneTag
        {
        };

    public:
        SceneView(const std::string& title, Frost::Scene* existingScene);
        SceneView(const std::filesystem::path& prefabPath);
        SceneView(const std::filesystem::path& meshPath, MeshPreviewTag);
        SceneView(const std::filesystem::path& scenePath, SceneTag);

        void OnUpdate(float deltaTime);
        void OnFixedUpdate(float deltaTime);

        void Draw(float deltaTime) override;

        void OnRenderHierarchy();
        void OnRenderInspector();

        const std::string& GetTitle() const { return _title; }
        bool IsFocused() const { return _isFocused; }
        bool IsOpen() const { return _isOpen; }

        Frost::Scene& GetScene() { return *_sceneContext; }
        const SceneViewSettings& GetSettings() const { return _viewSettings; }

    private:
        void _Init();
        void _ResizeViewportFramebuffer(uint32_t width, uint32_t height);

        // UI
        void _DrawToolbar();
        void _DrawEntityNode(entt::entity entityID);
        bool _DrawVec3Control(const std::string& label,
                              float* values,
                              float resetValue = 0.0f,
                              float columnWidth = 100.0f);

        // Actions
        void _SavePrefab();
        void _SaveScene();
        void _LoadScene();
        void _ReparentEntity(entt::entity entity, entt::entity newParent);
        void _FocusCameraOnEntity(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds);

        // Input
        void _HandleMeshDrop(const std::filesystem::path& meshPath);
        Frost::Math::Vector3 _GetSpawnPositionFromMouse();
        std::pair<Frost::Math::Vector3, Frost::Math::Vector3> _GetCameraRay(float mouseX,
                                                                            float mouseY,
                                                                            float viewportW,
                                                                            float viewportH);

    private:
        std::string _title;
        bool _isOpen = true;
        bool _isReadOnly = false;
        bool _isFocused = false;
        bool _isHovered = false;

        Frost::Scene* _sceneContext = nullptr;
        std::unique_ptr<Frost::Scene> _localScene;
        Frost::GameObject _selection;

        std::shared_ptr<Frost::Texture> _viewportTexture;
        uint32_t _viewportWidth = 0, _viewportHeight = 0;

        Frost::GameObject _editorCamera;
        Frost::GameObject _editorLight;

        std::filesystem::path _assetPath;
        bool _isPrefabView = false;

        char _nameBuffer[256] = { 0 };

        EditorCameraController _cameraController;

        SceneViewToolbar _toolbar;
        SceneViewSettings _viewSettings;
        GizmoOperation _currentGizmoOp = GizmoOperation::Translate;

        std::unique_ptr<Gizmo> _gizmo;
    };
} // namespace Editor