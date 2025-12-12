#pragma once

#include <Frost.h>

#include "Vehicles/Moto.h"
#include "Vehicles/Plane.h"
#include "Vehicles/Boat.h"

#include <memory>
#include <vector>
#include <string>

namespace GameLogic
{
    class PlayerController : public Frost::Scripting::Script
    {
    private:
        void _SetVehicle(VehicleType type);
        void _ProcessInput();

        Frost::GameObject _hudLeft;
        Frost::GameObject _hudMiddle;
        Frost::GameObject _hudRight;

        size_t _currentIndex = 0;

        const std::vector<std::string> _imagePaths = { "assets/Player/bike.png",
                                                       "assets/Player/boat.png",
                                                       "assets/Player/plane.png" };

        void _ChangeImageRight();

        void _ChangeImageLeft();

        size_t _GetSelectedIndex() const { return _currentIndex; }
        void _InitializeHUD();
        void _UpdateHUDDisplay();

    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;
        void OnPreFixedUpdate(float fixedDeltaTime) override;

        std::unique_ptr<Moto> _moto;
        std::unique_ptr<Plane> _plane;
        std::unique_ptr<Boat> _boat;
    };
} // namespace GameLogic