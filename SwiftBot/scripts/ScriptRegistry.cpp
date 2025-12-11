#include "Frost/Scripting/Script.h"

#include "GameState/GameState.h"

#include "HUD/PauseScreen.h"
#include "HUD/SplashScreen.h"
#include "HUD/VictoryScreen.h"

#include "Materials/Terrain.h"
#include "Materials/Water.h"
#include "Materials/Billboard.h"
#include "Materials/Grass.h"

#include "CheckPoint/Checkpoint.h"

#include "Looping/Looping.h"

#include "Player/PlayerController.h"
#include "Player/PlayerSpringCamera.h"

#include "Portal/Portal.h"

#include "Samples/Rotate.h"

using ScriptCreateFunc = std::function<Frost::Scripting::Script*()>;
static std::map<std::string, ScriptCreateFunc> _scriptRegistry;

void
InitializeRegistry()
{
    // HUD
    // Pause Screen
    _scriptRegistry["PauseScreen"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::PauseScreen()); };

    // Splash Screen
    _scriptRegistry["SplashScreen"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::SplashScreen()); };

    // Victory Screen
    _scriptRegistry["VictoryScreen"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::VictoryScreen()); };

    // Materials
    _scriptRegistry["Terrain"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Terrain()); };
    _scriptRegistry["Water"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Water()); };
    _scriptRegistry["Billboard"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Billboard()); };
    _scriptRegistry["Grass"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Grass()); };

    // Looping
    _scriptRegistry["Looping"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Looping()); };

    // Checkpoint
    _scriptRegistry["Checkpoint"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::Checkpoint()); };

    // Player
    _scriptRegistry["PlayerController"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::PlayerController()); };
    _scriptRegistry["PlayerSpringCamera"] = []()
    { return static_cast<Frost::Scripting::Script*>(new GameLogic::PlayerSpringCamera()); };

    // Portal
    _scriptRegistry["Portal"] = []() { return static_cast<Frost::Scripting::Script*>(new GameLogic::Portal()); };

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