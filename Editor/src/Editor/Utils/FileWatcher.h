#pragma once
#include <filesystem>
#include <functional>
#include <thread>
#include <atomic>

namespace Editor
{
    class FileWatcher
    {
    public:
        using OnChangeCallback = std::function<void()>;

        FileWatcher(const std::filesystem::path& pathToWatch);
        ~FileWatcher();

        void SetCallback(const OnChangeCallback& callback);
        void Start();
        void Stop();

    private:
        void _WatchThreadFunc();

    private:
        std::filesystem::path _directory;
        OnChangeCallback _callback;
        std::thread _watcherThread;
        std::atomic<bool> _running;
        void* _dirHandle = nullptr;
    };
} // namespace Editor