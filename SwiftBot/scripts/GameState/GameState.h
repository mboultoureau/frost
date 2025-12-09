#pragma once

#include "Frost.h"
#include "Player/Vehicles/Vehicle.h"

#include <vector>
#include <memory>
#include <algorithm>

namespace GameLogic
{
    struct PlayerData
    {
        Frost::GameObject playerObject;
        int lapsCompleted = 0;
        VehicleType currentVehicle = VehicleType::BIKE;
        bool forceSpecificCameraPos = false;
        int checkpointNumber = 0;
    };

    class GameState
    {
    public:
        GameState(const GameState&) = delete;
        GameState& operator=(const GameState&) = delete;

        ~GameState() = default;

        static GameState& Get();
        PlayerData& GetPlayerData(Frost::GameObject playerId);

        static void Initialize(int totalLaps, std::vector<PlayerData> initialPlayers);
        static void Shutdown();

        bool IsInitialized() const { return _initialized; }

        bool Finished() const;

        // Lap
        int GetTotalLaps() const { return _totalLaps; }
        void SetTotalLaps(int laps) { _totalLaps = laps; }
        void SetPlayerLaps(Frost::GameObject playerId, int laps);
        int GetCheckpointsCount() const { return _checkpointsCount; }

        const std::vector<PlayerData>& GetPlayersData() const { return _playersData; }

    private:
        GameState();

    private:
        bool _initialized = false;
        int _totalLaps;
        int _checkpointsCount;
        std::vector<PlayerData> _playersData;

        static std::unique_ptr<GameState> s_Instance;
    };
} // namespace GameLogic