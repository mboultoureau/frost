#pragma once

#include "Frost.h"

namespace Editor
{
    class ProjectHubLayer : public Frost::Layer
    {
    public:
        ProjectHubLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnLateUpdate(float deltaTime) override;

        static LayerName GetStaticName() { return "ProjectHubLayer"; }

        void RemoveFromRecents(const std::string& path);

    private:
        void _RenderHubUI();
        void _RenderCreateProjectPopup();

        void _LoadRecents();
        void _SaveRecents();
        void _AddToRecents(const std::string& path);
        void _OpenProject(const std::string& path);

        void _ShowError(const std::string& msg);
        void _RenderErrorPopup();

    private:
        std::vector<std::string> _recentProjects;
        static constexpr const char* _recentsFilePath = "editor_recents.yaml";

        bool _showCreateModal = false;
        char _newProjectNameBuffer[256] = "MyNewGame";
        char _newProjectLocationBuffer[1024] = "";

        bool _showDirectoryWarning = false;
        bool _showErrorModal = false;
        std::string _errorMessage = "";
    };
} // namespace Editor
