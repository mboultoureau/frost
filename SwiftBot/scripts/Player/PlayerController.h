#pragma once

#include <Frost.h>

#include "Vehicles/Moto.h"
#include "Vehicles/Plane.h"
#include "Vehicles/Boat.h"

#include <memory>

namespace GameLogic
{
    class PlayerController : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;
        void OnPreFixedUpdate(float fixedDeltaTime) override;

    private:
        void _SetVehicle(VehicleType type);
        void _ProcessInput();

    public:
        Frost::GameObject _motoObject;
        Frost::GameObject _boatObject;
        Frost::GameObject _planeObject;

        std::unique_ptr<Moto> _moto;
        std::unique_ptr<Plane> _plane;
        std::unique_ptr<Boat> _boat;
    };
} // namespace GameLogic
