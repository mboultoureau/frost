#include "PlatformUtils.h"
#include "Editor/EditorApp.h"
#include "Frost.h"

#ifdef FT_PLATFORM_WINDOWS
#include <commdlg.h>
#include <shellapi.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <windows.h>
#endif

using namespace Frost;

namespace Editor
{
    std::string FileDialogs::OpenFile(const char* filter)
    {
#ifdef FT_PLATFORM_WINDOWS
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);

        auto window = static_cast<WindowWin*>(Application::Get().GetWindow());
        ofn.hwndOwner = window->GetWindowHandle();

        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            return std::string(ofn.lpstrFile);
        }
#endif
        return std::string();
    }

    std::string FileDialogs::SaveFile(const char* filter)
    {
#ifdef FT_PLATFORM_WINDOWS
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);

        auto window = static_cast<WindowWin*>(Application::Get().GetWindow());
        ofn.hwndOwner = window->GetWindowHandle();

        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetSaveFileNameA(&ofn) == TRUE)
        {
            return std::string(ofn.lpstrFile);
        }
#endif
        return std::string();
    }

    std::string FileDialogs::PickFolder(const std::string& initialPath)
    {
#ifdef FT_PLATFORM_WINDOWS
        std::string result = "";
        IFileDialog* pfd = NULL;

        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        {
            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
            }

            if (!initialPath.empty())
            {
                IShellItem* psi = NULL;
                std::wstring wInitialPath(initialPath.begin(), initialPath.end());
                SHCreateItemFromParsingName(wInitialPath.c_str(), NULL, IID_PPV_ARGS(&psi));
                if (psi)
                {
                    pfd->SetFolder(psi);
                    psi->Release();
                }
            }

            if (SUCCEEDED(pfd->Show(NULL)))
            {
                IShellItem* psiResult;
                if (SUCCEEDED(pfd->GetResult(&psiResult)))
                {
                    PWSTR pszFilePath = NULL;
                    if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                    {
                        std::wstring ws(pszFilePath);
                        result = std::string(ws.begin(), ws.end());
                        CoTaskMemFree(pszFilePath);
                    }
                    psiResult->Release();
                }
            }
            pfd->Release();
        }

        return result;
#endif
        return std::string();
    }

    void FileDialogs::OpenInExplorer(const std::string& path)
    {
#ifdef FT_PLATFORM_WINDOWS
        std::string arg = "/select," + path;
        ShellExecuteA(NULL, "open", "explorer.exe", arg.c_str(), NULL, SW_SHOWDEFAULT);
#endif
    }
} // namespace Editor