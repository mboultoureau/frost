#pragma once

#include "Editor/EditorLayer.h"
#include "Editor/Project/ProjectInfo.h"
#include "Editor/Project/ProjectOpenEvent.h"
#include "Editor/ProjectHubLayer.h"

#include "Frost.h"

namespace Editor
{
    class EditorApp : public Frost::Application
    {
    public:
        EditorApp(Frost::ApplicationEntryPoint entryPoint);
        ~EditorApp();

        void OnApplicationReady() override;

        bool OnProjectOpen(ProjectOpenEvent& e);
        const ProjectInfo& GetProjectInfo() const { return _projectInfo; }

    private:
        ProjectHubLayer* _projectHubLayer;
        EditorLayer* _editorLayer;
        ProjectInfo _projectInfo;

        Frost::EventHandlerId _projectOpenEventHandlerId;
    };
} // namespace Editor