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
        VehicleType currentVehicle = VehicleType::MOTO;
        bool forceSpecificCameraPos = false;
        int checkpointNumber = 0;
        bool isInWater = false;
        bool isInToon = false;
        bool isInAntiGravity = false;
        bool isInGrass = false;
        bool isInHandWritting = false;
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
        bool IsPlayer(Frost::GameObject playerObject) const;

        bool Finished() const;

        // Lap
        int GetTotalLaps() const { return _totalLaps; }
        void SetTotalLaps(int laps) { _totalLaps = laps; }
        void SetPlayerLaps(Frost::GameObject playerId, int laps);
        int GetCheckpointsCount() const { return _checkpointsCount; }

        // Vehicle
        void SetPlayerVehicle(Frost::GameObject playerId, VehicleType type);

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