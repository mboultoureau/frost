#include "Editor/Utils/FileDialogs.h"

#include "Frost/Core/Application.h"
#include "Frost/Core/Windows/WindowWin.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>

namespace Editor
{
    HWND GetOwnerWindow()
    {
        auto* window = Frost::Application::GetWindow();
        auto* winWindow = static_cast<Frost::WindowWin*>(window);
        return winWindow ? winWindow->GetWindowHandle() : NULL;
    }

    std::optional<std::string> FileDialogs::OpenFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = GetOwnerWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::nullopt;
    }

    std::optional<std::string> FileDialogs::SaveFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = GetOwnerWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        const char* p = filter;
        while (*p)
            p++;
        p++;
        if (*p == '*' && *(p + 1) == '.')
            ofn.lpstrDefExt = p + 2;

        if (GetSaveFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::nullopt;
    }
} // namespace Editor