#include "Editor/EditorApp.h"

#include "Frost/Core/EntryPoint.h"
#include "Frost/Core/Layer.h"

using namespace Frost;

namespace Editor
{
    EditorApp* EditorApp::_singleton = nullptr;

    EditorApp::EditorApp(Frost::ApplicationEntryPoint entryPoint) : Frost::Application(entryPoint)
    {
        FT_ENGINE_ASSERT(!_singleton, "EditorApp already exists!");
        _singleton = this;
    }

    EditorApp::~EditorApp()
    {
        _singleton = nullptr;

        EventManager::Unsubscribe<ProjectOpenEvent>(_projectOpenEventHandlerId);
        EventManager::Unsubscribe<ProjectCloseEvent>(_projectCloseEventHandlerId);
    }

    void EditorApp::OnApplicationReady()
    {
        _projectOpenEventHandlerId =
            EventManager::Subscribe<ProjectOpenEvent>(FROST_BIND_EVENT_FN(EditorApp::OnProjectOpen));
        _projectCloseEventHandlerId =
            EventManager::Subscribe<ProjectCloseEvent>(FROST_BIND_EVENT_FN(EditorApp::OnProjectClose));

        _projectHubLayer = PushLayer<Editor::ProjectHubLayer>();
    }

    bool EditorApp::OnProjectOpen(ProjectOpenEvent& e)
    {
        std::string projectPath = e.GetProjectPath();
        bool loaded = _projectInfo.LoadFromPath(projectPath);

        if (!loaded)
        {
            FT_ENGINE_ERROR("Failed to load project from path: {0}", projectPath);

            if (_projectHubLayer)
            {
                _projectHubLayer->RemoveFromRecents(projectPath);
            }

            // Display error to user
            std::string errorMessage = "Failed to load project from path:\n" + projectPath;
            MessageBoxA(nullptr, errorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);

            return true;
        }

        FT_ENGINE_INFO(
            "Project '{0}' loaded from path: {1}", _projectInfo.GetConfig().name, _projectInfo.GetProjectFilePath());

        PopLayer(_projectHubLayer);
        _projectHubLayer = nullptr;

        Frost::Application::SetProjectDirectory(_projectInfo.GetProjectDir());
        _editorLayer = PushLayer<Editor::EditorLayer>();

        return true;
    }

    bool EditorApp::OnProjectClose(ProjectCloseEvent& e)
    {
        if (_editorLayer)
        {
            PopLayer(_editorLayer);
            _editorLayer = nullptr;
        }

        _projectInfo.Clear();

        if (!_projectHubLayer)
        {
            _projectHubLayer = PushLayer<Editor::ProjectHubLayer>();
        }

        Frost::Application::SetProjectDirectory(".");
        FT_ENGINE_INFO("Project closed. Returning to Project Hub.");

        return true;
    }
} // namespace Editor

Frost::Application*
Frost::CreateApplication(ApplicationEntryPoint entryPoint)
{
    entryPoint.title = Window::WindowTitle{ L"Editor" };
    return new Editor::EditorApp(entryPoint);
}
