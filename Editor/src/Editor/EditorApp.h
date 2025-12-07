#pragma once

#include "Editor/EditorLayer.h"
#include "Editor/Project/ProjectInfo.h"
#include "Editor/Events/ProjectOpenEvent.h"
#include "Editor/Events/ProjectCloseEvent.h"
#include "Editor/ProjectHubLayer.h"

#include "Frost.h"

namespace Editor
{
    class EditorApp : public Frost::Application
    {
    public:
        static EditorApp& Get() { return *_singleton; }

    public:
        EditorApp(Frost::ApplicationSpecification entryPoint);
        ~EditorApp();

        void OnApplicationReady() override;

        bool OnProjectOpen(ProjectOpenEvent& e);
        bool OnProjectClose(ProjectCloseEvent& e);
        const ProjectInfo& GetProjectInfo() const { return _projectInfo; }
        EditorLayer* GetEditorLayer() { return _editorLayer; }

    private:
        static EditorApp* _singleton;

        ProjectHubLayer* _projectHubLayer;
        EditorLayer* _editorLayer;
        ProjectInfo _projectInfo;

        Frost::EventHandlerId _projectOpenEventHandlerId;
        Frost::EventHandlerId _projectCloseEventHandlerId;
    };
} // namespace Editor