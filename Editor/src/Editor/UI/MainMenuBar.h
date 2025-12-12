#pragma once

#include "Editor/UI/EditorPanel.h"
#include "Editor/Project/ProjectInfo.h"

namespace Editor
{
    class MainMenuBar : public EditorPanel
    {
    public:
        MainMenuBar(const ProjectInfo& projectInfo);
        ~MainMenuBar() = default;

        void Draw(float deltaTime) override;

    private:
        void _RenderFileMenu();
        void _RenderEditMenu();
        void _RenderBuildMenu();

        void _RenderProjectName();
        void _CompileAllAssets();

    private:
        const ProjectInfo& _projectInfo;
    };
} // namespace Editor