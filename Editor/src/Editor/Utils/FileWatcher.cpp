#include "Editor/Utils/FileWatcher.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace Editor
{
    FileWatcher::FileWatcher(const std::filesystem::path& pathToWatch) : _directory(pathToWatch), _running(false) {}

    FileWatcher::~FileWatcher()
    {
        Stop();
    }

    void FileWatcher::SetCallback(const OnChangeCallback& callback)
    {
        _callback = callback;
    }

    void FileWatcher::Start()
    {
        Stop();
        _running = true;
        _watcherThread = std::thread(&FileWatcher::_WatchThreadFunc, this);
    }

    void FileWatcher::Stop()
    {
        _running = false;

        if (_dirHandle && _dirHandle != INVALID_HANDLE_VALUE)
        {
            CancelIoEx((HANDLE)_dirHandle, nullptr);
            CloseHandle((HANDLE)_dirHandle);
            _dirHandle = nullptr;
        }

        if (_watcherThread.joinable())
        {
            _watcherThread.join();
        }
    }

    void FileWatcher::_WatchThreadFunc()
    {
        _dirHandle = CreateFileW(_directory.c_str(),
                                 FILE_LIST_DIRECTORY,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_BACKUP_SEMANTICS,
                                 NULL);

        if (_dirHandle == INVALID_HANDLE_VALUE)
            return;

        char buffer[4096];
        DWORD bytesReturned;

        while (_running)
        {
            if (ReadDirectoryChangesW((HANDLE)_dirHandle,
                                      buffer,
                                      sizeof(buffer),
                                      TRUE,
                                      FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                          FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                                      &bytesReturned,
                                      NULL,
                                      NULL))
            {
                if (bytesReturned > 0 && _callback)
                {
                    // debounce multiple events
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    _callback();
                }
            }
            else
            {
                break;
            }
        }
    }
} // namespace Editor