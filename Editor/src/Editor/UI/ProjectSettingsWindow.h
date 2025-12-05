#pragma once

#include "Editor/UI/EditorPanel.h"
#include "Editor/Project/ProjectInfo.h"

namespace Editor
{
    class ProjectSettingsWindow : public EditorPanel
    {
    public:
        ProjectSettingsWindow(ProjectInfo& projectInfo);

        void Draw(float deltaTime) override;

        bool IsOpen() const { return _isOpen; }
        void Open() { _isOpen = true; }
        void Close() { _isOpen = false; }

        static const char* GetStaticTitle() { return "Project Settings"; }

    private:
        // Render
        void _RenderLeftPane();
        void _RenderRightPane();

        void _RenderGeneralSettings();
        void _RenderPhysicsSettings();

        template<typename TLayer>
        void _RenderCollisionMatrix(const std::vector<TLayer>& layers, std::vector<bool>& matrix);

        // Actions
        void _AddBroadPhaseLayer();
        void _RemoveBroadPhaseLayer(int indexToRemove);

        void _AddObjectLayer();
        void _RemoveObjectLayer(int indexToRemove);

    private:
        ProjectInfo& _projectInfo;
        ProjectConfig _editableConfig;
        bool _isOpen = false;

        enum class ESettingsCategory
        {
            General,
            Physics
        };

        ESettingsCategory _currentCategory = ESettingsCategory::General;
    };
} // namespace Editor