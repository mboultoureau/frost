#include "Frost/Scripting/Script.h"

#include "HUD/SplashScreen.h"
#include "Materials/Terrain.h"
#include "Materials/Water.h"
#include "Player/PlayerController.h"
#include "Samples/Rotate.h"

using ScriptCreateFunc = std::function<Frost::Scripting::Script*()>;
static std::map<std::string, ScriptCreateFunc> _scriptRegistry;

void
InitializeRegistry()
{
    // HUD
    _scriptRegistry["SplashScreen"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::SplashScreen()); };

    // Materials
    _scriptRegistry["Terrain"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Terrain()); };
    _scriptRegistry["Water"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Water()); };

    // Player
    _scriptRegistry["PlayerController"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::PlayerController()); };

    // Samples
    _scriptRegistry["Rotate"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Rotate()); };
}

struct ScriptNameCollection
{
    int count;
    const char** names;
};

static std::vector<const char*> _scriptNamesCache;
static ScriptNameCollection _scriptNameCollection;

#define EXPORT_API __declspec(dllexport)
extern "C"
{
    EXPORT_API Frost::Scripting::Script* CreateScript(const char* name)
    {
        if (_scriptRegistry.empty())
            InitializeRegistry();

        if (_scriptRegistry.count(name))
            return _scriptRegistry[name]();

        return nullptr;
    }

    EXPORT_API ScriptNameCollection GetAvailableScripts()
    {
        if (_scriptRegistry.empty())
            InitializeRegistry();

        if (_scriptNamesCache.empty() && !_scriptRegistry.empty())
        {
            _scriptNamesCache.reserve(_scriptRegistry.size());
            for (const auto& pair : _scriptRegistry)
            {
                _scriptNamesCache.push_back(pair.first.c_str());
            }

            _scriptNameCollection.names = _scriptNamesCache.data();
            _scriptNameCollection.count = static_cast<int>(_scriptNamesCache.size());
        }

        return _scriptNameCollection;
    }
}