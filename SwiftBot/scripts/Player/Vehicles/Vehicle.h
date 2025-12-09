#pragma once

#include "Frost.h"

namespace GameLogic
{
    class Vehicle
    {
    public:
        Vehicle(Frost::GameObject _player, Frost::GameObject vehicle) : _player(_player), _vehicle(vehicle) {}
        virtual ~Vehicle() = default;

        virtual void Show() {}
        virtual void Hide() {}

    protected:
        Frost::GameObject _player;
        Frost::GameObject _vehicle;
    };
} // namespace GameLogic