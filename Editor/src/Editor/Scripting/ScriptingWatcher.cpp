#include "Editor/Scripting/ScriptingWatcher.h"
#include "Frost/Debugging/Logger.h"

using namespace Frost;

namespace Editor
{
    ScriptingWatcher::ScriptingWatcher() = default;

    ScriptingWatcher::~ScriptingWatcher()
    {
        Stop();
    }

    void ScriptingWatcher::SetPathToWatch(const std::filesystem::path& dllPath)
    {
        _dllPath = dllPath;
        if (std::filesystem::exists(_dllPath))
        {
            _lastWriteTime = std::filesystem::last_write_time(_dllPath);
        }
    }

    void ScriptingWatcher::Start()
    {
        if (_running)
            return;

        if (_dllPath.empty() || !std::filesystem::exists(_dllPath.parent_path()))
        {
            FT_WARN("ScriptingWatcher: Path to watch is not set or directory does not exist. Watcher not started.");
            return;
        }

        _running = true;
        _reloadRequested = false;
        _watcherThread = std::thread(&ScriptingWatcher::_WatchThreadFunc, this);
        FT_INFO("ScriptingWatcher started for: {}", _dllPath.string());
    }

    void ScriptingWatcher::Stop()
    {
        _running = false;
        if (_watcherThread.joinable())
        {
            _watcherThread.join();
        }
    }

    bool ScriptingWatcher::ReloadRequested()
    {
        return _reloadRequested.exchange(false);
    }

    void ScriptingWatcher::_WatchThreadFunc()
    {
        while (_running)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            if (!std::filesystem::exists(_dllPath))
            {
                continue;
            }

            auto currentWriteTime = std::filesystem::last_write_time(_dllPath);

            if (currentWriteTime != _lastWriteTime)
            {
                FT_ENGINE_INFO("ScriptingWatcher: DLL change detected! Requesting hot-reload.");
                _lastWriteTime = currentWriteTime;
                _reloadRequested = true;
            }
        }
    }
} // namespace Editor