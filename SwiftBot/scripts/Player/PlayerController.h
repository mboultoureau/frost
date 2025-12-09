#pragma once

#include <Frost.h>

namespace GameLogic
{
    class PlayerController : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;

    public:
        enum VehicleType
        {
            BIKE = 0,
            BOAT = 1,
            PLANE = 2
        };

        Frost::GameObject _bike;
        Frost::GameObject _boat;
        Frost::GameObject _plane;

    private:
    };
} // namespace GameLogic
