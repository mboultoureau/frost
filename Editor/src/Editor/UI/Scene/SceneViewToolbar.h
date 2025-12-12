#pragma once

#include "Frost/Asset/Texture.h"
#include "Editor/Utils/EditorCameraController.h"

#include <memory>
#include <functional>
#include <map>
#include <string>

namespace Editor
{
    struct SceneViewSettings
    {
        bool showEditorSkybox = true;
        bool showEditorLight = true;
        bool showWireframe = false;
        bool showGrid = true;
        bool enableLighting = true;
        bool showPostProcess = true;
    };

    enum class GizmoOperation
    {
        None = -1,
        Translate = 0,
        Rotate = 1,
        Scale = 2
    };

    class SceneViewToolbar
    {
    public:
        SceneViewToolbar();
        ~SceneViewToolbar() = default;

        void Init();

        void Draw(EditorCameraController& cameraController,
                  GizmoOperation& currentGizmoOp,
                  SceneViewSettings& viewSettings,
                  bool isPrefabView,
                  const std::function<void()>& onSavePrefabCallback,
                  const std::function<void()>& onSaveSceneCallback,
                  const std::function<void()>& onLoadSceneCallback);

    private:
        std::shared_ptr<Frost::Texture> _GetIcon(const std::string& name);
        bool _DrawToggleButton(const std::string& id, std::shared_ptr<Frost::Texture> icon, bool selected);

    private:
        std::map<std::string, std::shared_ptr<Frost::Texture>> _icons;
        float _buttonSize = 24.0f;
    };
} // namespace Editor