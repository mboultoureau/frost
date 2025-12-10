#pragma once

#include "Frost.h"

namespace GameLogic
{
    enum class VehicleType
    {
        MOTO = 0,
        BOAT = 1,
        PLANE = 2,
        COUNT
    };

    class Vehicle
    {
    public:
        Vehicle(Frost::GameObject player);
        virtual ~Vehicle() = default;

        virtual void Show() {}
        virtual void Hide() {}

        virtual void OnPreFixedUpdate(float fixedDeltaTime) {}
        virtual void OnFixedUpdate(float fixedDeltaTime) {}

        virtual void OnBrake(bool brake) {}
        virtual void OnSpecialAction(bool special) {}
        virtual void OnMove(float right, float forward) {}

    protected:
        Frost::GameObject _player;
        Frost::GameObject _playerController;
        Frost::GameObject _camera;
        Frost::GameObject _vehicle;
    };
} // namespace GameLogic