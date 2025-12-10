#include "Player/Vehicles/Vehicle.h"

namespace GameLogic
{
    Vehicle::Vehicle(Frost::GameObject player) :
        _player{ player },
        _playerController{ player.GetChildByName("PlayerController", true) },
        _camera{ player.GetChildByName("Camera", true) }
    {
    }
} // namespace GameLogic