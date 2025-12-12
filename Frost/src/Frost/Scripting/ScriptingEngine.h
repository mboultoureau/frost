#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Scripting/ScriptingBridge.h"
#include "Frost/Utils/NoCopy.h"

#include <string>
#include <vector>

namespace Frost::Scripting
{
    class Script;

    class FROST_API ScriptingEngine : NoCopy
    {
    public:
        static ScriptingEngine& GetInstance();

        static void LoadScriptingDLL(const std::string& path);
        static void UnloadScriptingDLL();

        static Script* CreateScript(const std::string& name);
        static std::vector<std::string> GetAvailableScripts();

    private:
        ScriptingEngine() = default;
        ~ScriptingEngine();

        void* _dllHandle = nullptr;

        using CreateScriptFunc = Script* (*)(const char*);
        using GetScriptsFunc = ScriptNameCollection (*)();
        ;

        CreateScriptFunc _createFunc = nullptr;
        GetScriptsFunc _getScriptsFunc = nullptr;
    };
} // namespace Frost::Scripting