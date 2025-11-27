#pragma once

#include "Frost.h"

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

private:
    int _lap;
    int _totalLap;
};