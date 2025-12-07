#pragma once

#include <filesystem>
#include <thread>
#include <atomic>
#include <functional>

namespace Editor
{
    class ScriptingWatcher
    {
    public:
        ScriptingWatcher();
        ~ScriptingWatcher();

        void SetPathToWatch(const std::filesystem::path& dllPath);

        void Start();
        void Stop();

        bool ReloadRequested();

    private:
        void _WatchThreadFunc();

        std::filesystem::path _dllPath;
        std::filesystem::file_time_type _lastWriteTime;

        std::thread _watcherThread;
        std::atomic<bool> _running = false;
        std::atomic<bool> _reloadRequested = false;
    };
} // namespace Editor