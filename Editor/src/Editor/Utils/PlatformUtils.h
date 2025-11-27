#pragma once
#include <string>

namespace Editor
{
    class FileDialogs
    {
    public:
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter);
        static std::string PickFolder(const std::string& initialPath = "");
        static void OpenInExplorer(const std::string& path);
    };
} // namespace Editor