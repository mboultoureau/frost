#pragma once

#include "Editor/UI/EditorPanel.h"
#include "Editor/Utils/EditorCameraController.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Renderer/BoundingBox.h"

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

    public:
        SceneView(const std::string& title, Frost::Scene* existingScene);
        SceneView(const std::filesystem::path& prefabPath);
        SceneView(const std::filesystem::path& meshPath, MeshPreviewTag);

        void OnUpdate(float deltaTime);

        void Draw() override;
        void DrawViewport();

        void OnRenderHierarchy();
        void OnRenderInspector();

        const std::string& GetTitle() const { return _title; }
        bool IsFocused() const { return _isFocused; }
        bool IsOpen() const { return _isOpen; }

    private:
        void _Init();
        void _ResizeViewportFramebuffer(uint32_t width, uint32_t height);
        void _DrawToolbar();

        bool _DrawVec3Control(const std::string& label,
                              float* values,
                              float resetValue = 0.0f,
                              float columnWidth = 100.0f);

        void _DrawEntityNode(entt::entity entityID);
        void _ReparentEntity(entt::entity entity, entt::entity newParent);
        void _FocusCameraOnEntity(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds);

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
        std::filesystem::path _assetPath;
        bool _isPrefabView = false;

        char _nameBuffer[256] = { 0 };

        EditorCameraController _cameraController;
    };
} // namespace Editor
