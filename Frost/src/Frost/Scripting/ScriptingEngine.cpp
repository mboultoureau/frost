#include "Frost/Scripting/ScriptingEngine.h"
#include "Frost/Debugging/Logger.h"

#ifdef FT_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#error "ScriptingEngine currently only supports Windows."
#endif

namespace Frost::Scripting
{
    ScriptingEngine::~ScriptingEngine()
    {
        UnloadScriptingDLL();
    }

    ScriptingEngine& ScriptingEngine::GetInstance()
    {
        static ScriptingEngine instance;
        return instance;
    }

    void ScriptingEngine::LoadScriptingDLL(const std::string& path)
    {
        auto& engine = GetInstance();

        if (engine._dllHandle)
        {
            UnloadScriptingDLL();
        }

        engine._dllHandle = LoadLibraryA(path.c_str());
        if (!engine._dllHandle)
        {
            DWORD errorCode = GetLastError();
            FT_ENGINE_ERROR("Failed to load scripting DLL from path '{}'. Error code: {}", path, errorCode);
            return;
        }

        engine._createFunc =
            reinterpret_cast<CreateScriptFunc>(GetProcAddress(static_cast<HMODULE>(engine._dllHandle), "CreateScript"));
        engine._getScriptsFunc = reinterpret_cast<GetScriptsFunc>(
            GetProcAddress(static_cast<HMODULE>(engine._dllHandle), "GetAvailableScripts"));

        if (!engine._createFunc || !engine._getScriptsFunc)
        {
            FT_ENGINE_ERROR("Failed to load required functions from scripting DLL at path '{}'.", path);
            UnloadScriptingDLL();
            return;
        }

        FT_ENGINE_INFO("Successfully loaded scripting DLL from path '{}'.", path);
    }

    void ScriptingEngine::UnloadScriptingDLL()
    {
        auto& engine = GetInstance();

        if (!engine._dllHandle)
        {
            return;
        }

        if (!FreeLibrary(static_cast<HMODULE>(engine._dllHandle)))
        {
            DWORD errorCode = GetLastError();
            FT_ENGINE_ERROR("Failed to unload scripting DLL. Error code: {}", errorCode);
            return;
        }

        FT_ENGINE_INFO("Successfully unloaded scripting DLL.");

        engine._dllHandle = nullptr;
        engine._createFunc = nullptr;
        engine._getScriptsFunc = nullptr;
    }

    Script* ScriptingEngine::CreateScript(const std::string& name)
    {
        auto& engine = GetInstance();

        if (!engine._createFunc)
        {
            FT_ENGINE_ERROR("CreateScript function is not loaded.");
            return nullptr;
        }

        return engine._createFunc(name.c_str());
    }

    std::vector<std::string> ScriptingEngine::GetAvailableScripts()
    {
        auto& engine = GetInstance();
        std::vector<std::string> availableScripts;

        if (!engine._getScriptsFunc)
        {
            FT_ENGINE_ERROR("GetAvailableScripts function is not loaded.");
            return availableScripts;
        }

        ScriptNameCollection collection = engine._getScriptsFunc();
        for (int i = 0; i < collection.count; ++i)
        {
            availableScripts.push_back(collection.names[i]);
        }

        return availableScripts;
    }
} // namespace Frost::Scripting