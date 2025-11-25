#include "Editor/EditorApp.h"

#include "Frost/Core/EntryPoint.h"

using namespace Frost;

namespace Editor
{
	EditorApp::EditorApp(Frost::ApplicationEntryPoint entryPoint) : Frost::Application(entryPoint)
	{
	}

	EditorApp::~EditorApp()
	{
		EventManager::Unsubscribe<ProjectOpenEvent>(_projectOpenEventHandlerId);
	}

	void EditorApp::OnApplicationReady()
	{
		_projectOpenEventHandlerId = EventManager::Subscribe<ProjectOpenEvent>(FROST_BIND_EVENT_FN(EditorApp::OnProjectOpen));
		_projectHubLayer = PushLayer<Editor::ProjectHubLayer>();
	}

	bool EditorApp::OnProjectOpen(ProjectOpenEvent& e)
	{
		std::string projectPath = e.GetProjectPath();
		bool loaded = _projectInfo.LoadFromPath(projectPath);
		
		if (!loaded)
		{
			FT_ENGINE_ERROR("Failed to load project from path: {0}", projectPath);

			// Display error to user
			std::string errorMessage = "Failed to load project from path:\n" + projectPath;
			MessageBoxA(nullptr, errorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);

			return true;
		}


		FT_ENGINE_INFO("Project '{0}' loaded from path: {1}", _projectInfo.GetConfig().name, _projectInfo.GetProjectFilePath());

		PopLayer(_projectHubLayer);
		PushLayer<Editor::EditorLayer>();

		return true;
	}
}

Frost::Application* Frost::CreateApplication(ApplicationEntryPoint entryPoint)
{
	entryPoint.title = Window::WindowTitle{ L"Editor" };
	return new Editor::EditorApp(entryPoint);
}
