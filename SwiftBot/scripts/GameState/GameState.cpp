#include "GameState/GameState.h"

#include <algorithm>
#include <Frost/Debugging/Logger.h>

using namespace Frost;

namespace GameLogic
{

    std::unique_ptr<GameState> GameState::s_Instance = nullptr;

    GameState::GameState() : _totalLaps{ 1 }, _checkpointsCount{ 3 }, _initialized{ false } {}

    GameState& GameState::Get()
    {
        if (!s_Instance)
        {
            s_Instance = std::unique_ptr<GameState>(new GameState());
        }
        return *s_Instance;
    }

    PlayerData& GameState::GetPlayerData(Frost::GameObject playerId)
    {
        auto it = std::find_if(_playersData.begin(),
                               _playersData.end(),
                               [&playerId](const PlayerData& data) { return data.playerObject == playerId; });
        if (it != _playersData.end())
        {
            return *it;
        }
        else
        {
            FT_ASSERT(false, "Player not found in GameState when getting player data.");
            throw std::runtime_error("Player not found in GameState when getting player data.");
        }
    }

    void GameState::Initialize(int totalLaps, std::vector<PlayerData> initialPlayers)
    {
        if (s_Instance)
        {
            FT_WARN("GameState already initialized. Reinitializing.");
            s_Instance.reset();
        }

        s_Instance = std::unique_ptr<GameState>(new GameState());
        s_Instance->_totalLaps = totalLaps;
        s_Instance->_playersData = std::move(initialPlayers);
        s_Instance->_initialized = true;
    }

    void GameState::Shutdown()
    {
        s_Instance.reset();
    }

    bool GameState::Finished() const
    {
        return std::any_of(_playersData.begin(),
                           _playersData.end(),
                           [this](const PlayerData& data) { return data.lapsCompleted >= _totalLaps; });
    }

    void GameState::SetPlayerLaps(Frost::GameObject playerId, int laps)
    {
        auto it = std::find_if(_playersData.begin(),
                               _playersData.end(),
                               [&playerId](const PlayerData& data) { return data.playerObject == playerId; });

        if (it != _playersData.end())
        {
            it->lapsCompleted = laps;
        }
        else
        {
            FT_ASSERT(false, "Player not found in GameState when setting laps.");
        }
    }

    void GameState::SetPlayerVehicle(Frost::GameObject playerId, VehicleType type)
    {
        auto it = std::find_if(_playersData.begin(),
                               _playersData.end(),
                               [&playerId](const PlayerData& data) { return data.playerObject == playerId; });

        if (it != _playersData.end())
        {
            it->currentVehicle = type;
        }
        else
        {
            FT_ASSERT(false, "Player not found in GameState when setting vehicle type.");
        }
    }

    bool GameState::IsPlayer(Frost::GameObject playerObject) const
    {
        return std::any_of(_playersData.begin(),
                           _playersData.end(),
                           [&](const PlayerData& data) { return data.playerObject == playerObject; });
    }
} // namespace GameLogic
