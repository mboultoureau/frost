#pragma once

#include "Frost.h"
#include "Player/Player.h"
using namespace Frost;

class GameState
{
public:
    GameState() { _totalLap = _lap = 0; };
    void SetLap(int lap)
    {
        _lap = 0;
        _totalLap = lap;
    };
    void IncreaseLap() { _lap++; };
    bool Finished() { return _lap >= _totalLap; };
    void ResetLap() { _lap = 0; };

    std::vector<Player> GetPlayers() { return _playersList; };
    bool IsGameStarted() { return _isGameStarted; };
    void StartGame() { _isGameStarted = true; };

private:
    bool _isGameStarted = false;
    std::vector<Player> _playersList;
    int _lap;
    int _totalLap;
};