#pragma once

#include "Frost.h"

#include <string>

namespace Editor
{
    class ProjectOpenEvent : public Frost::Event
    {
    public:
        ProjectOpenEvent(const std::string& path) : _projectPath(path) {}

        std::string GetProjectPath() const { return _projectPath; }

        Frost::EventType GetEventType() const override { return GetStaticType(); }
        std::string ToString() const override { return "ProjectOpenEvent"; }

        static Frost::EventType GetStaticType() { return Frost::EventType::AppCustom; }

    private:
        std::string _projectPath;
    };
} // namespace Editor