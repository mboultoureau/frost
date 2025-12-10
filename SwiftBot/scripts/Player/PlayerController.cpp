#include "Player/PlayerController.h"
#include "GameState/GameState.h"

#include <iostream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void PlayerController::OnCreate()
    {
        // TODO: Image manager

        _moto = std::make_unique<Moto>(_gameObject);
        _boat = std::make_unique<Boat>(_gameObject);
        _plane = std::make_unique<Plane>(_gameObject);

        std::vector<PlayerData> initialPlayers = {
            PlayerData{ .playerObject = GetGameObject(), .lapsCompleted = 0 },
        };
        GameState::Get().Initialize(0, initialPlayers);
        GameState::Get().SetPlayerLaps(GetGameObject(), 0);

        _SetVehicle(VehicleType::MOTO);
    }

    void PlayerController::OnUpdate(float deltaTime)
    {
        _ProcessInput();
    }

    void PlayerController::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!GameState::Get().IsInitialized())
            return;

        const auto& playerData = GameState::Get().GetPlayerData(GetGameObject());
        auto currentVehicleType = playerData.currentVehicle;

        switch (currentVehicleType)
        {
            case VehicleType::MOTO:
                _moto->OnFixedUpdate(fixedDeltaTime);
                break;
            case VehicleType::BOAT:
                _boat->OnFixedUpdate(fixedDeltaTime);
                break;
            case VehicleType::PLANE:
                _plane->OnFixedUpdate(fixedDeltaTime);
                break;
        }
    }

    void PlayerController::OnPreFixedUpdate(float fixedDeltaTime)
    {
        if (!GameState::Get().IsInitialized())
            return;

        // Update current vehicle
        auto& playerData = GameState::Get().GetPlayerData(GetGameObject());
        auto currentVehicleType = playerData.currentVehicle;

        switch (currentVehicleType)
        {
            case VehicleType::MOTO:
                _moto->OnPreFixedUpdate(fixedDeltaTime);
                break;
            case VehicleType::BOAT:
                _boat->OnPreFixedUpdate(fixedDeltaTime);
                break;
            case VehicleType::PLANE:
                _plane->OnPreFixedUpdate(fixedDeltaTime);
                break;
        }
    }

    void PlayerController::_SetVehicle(VehicleType type)
    {
        if (!GameState::Get().IsInitialized())
            return;

        // Set GameState
        auto& playerData = GameState::Get().GetPlayerData(GetGameObject());
        playerData.currentVehicle = type;

        // Destroy all vehicles
        _moto->Hide();
        _plane->Hide();
        _boat->Hide();

        // Show selected vehicle
        switch (type)
        {
            case VehicleType::MOTO:
                _moto->Show();
                break;
            case VehicleType::BOAT:
                _boat->Show();
                break;
            case VehicleType::PLANE:
                _plane->Show();
                break;
        }
    }

    // TODO: Support gamepad input
    void PlayerController::_ProcessInput()
    {
        auto& playerData = GameState::Get().GetPlayerData(GetGameObject());
        auto currentVehicleType = playerData.currentVehicle;

        // Switch to previous vehicle
        if (Input::GetKeyboard().IsKeyPressed(K_Q))
        {
            auto previousVehicleType = static_cast<VehicleType>(
                (static_cast<int>(currentVehicleType) - 1 + static_cast<int>(VehicleType::COUNT)) %
                static_cast<int>(VehicleType::COUNT));
            _SetVehicle(previousVehicleType);
        }
        // Switch to next vehicle
        if (Input::GetKeyboard().IsKeyPressed(K_E))
        {
            auto nextVehicleType = static_cast<VehicleType>((static_cast<int>(currentVehicleType) + 1) %
                                                            static_cast<int>(VehicleType::COUNT));
            _SetVehicle(nextVehicleType);
        }

        // Movement
        float right = 0.0f;
        float forward = 0.0f;

        if (Input::GetKeyboard().IsKeyDown(K_D))
        {
            right = 1.0f;
        }
        else if (Input::GetKeyboard().IsKeyDown(K_A))
        {
            right = -1.0f;
        }

        if (Input::GetKeyboard().IsKeyDown(K_W))
        {
            forward = 1.0f;
        }
        else if (Input::GetKeyboard().IsKeyDown(K_S))
        {
            forward = -1.0f;
        }

        switch (currentVehicleType)
        {
            case VehicleType::MOTO:
                _moto->OnMove(right, forward);
                break;
            case VehicleType::BOAT:
                _boat->OnMove(right, forward);
                break;
            case VehicleType::PLANE:
                _plane->OnMove(right, forward);
                break;
        }

        // Brake
        bool isBraking = Input::GetKeyboard().IsKeyPressed(K_SHIFT);
        switch (currentVehicleType)
        {
            case VehicleType::MOTO:
                _moto->OnBrake(isBraking);
                break;
            case VehicleType::BOAT:
                _boat->OnBrake(isBraking);
                break;
            case VehicleType::PLANE:
                _plane->OnBrake(isBraking);
                break;
        }

        // Special Action
        bool isSpecial = Input::GetKeyboard().IsKeyPressed(K_SPACE);
        switch (currentVehicleType)
        {
            case VehicleType::MOTO:
                _moto->OnSpecialAction(isSpecial);
                break;
            case VehicleType::BOAT:
                _boat->OnSpecialAction(isSpecial);
                break;
            case VehicleType::PLANE:
                _plane->OnSpecialAction(isSpecial);
                break;
        }
    }
} // namespace GameLogic
