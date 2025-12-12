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
        auto& prefabTransform = GetGameObject().GetParent().GetComponent<Transform>();
        auto& transform = GetGameObject().GetChildByName("PlayerController").GetComponent<Transform>();
        transform.position = prefabTransform.position;
        transform.rotation = prefabTransform.rotation;

        prefabTransform = Transform();

        _InitializeHUD();

        _currentIndex = 0;
        _UpdateHUDDisplay();

        _moto = std::make_unique<Moto>(_gameObject);
        _boat = std::make_unique<Boat>(_gameObject);
        _plane = std::make_unique<Plane>(_gameObject);

        std::vector<PlayerData> initialPlayers = {
            PlayerData{ .playerObject = GetGameObject(), .lapsCompleted = 0 },
        };
        GameState::Get().Initialize(1, initialPlayers);
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
            _ChangeImageLeft();
        }
        // Switch to next vehicle
        if (Input::GetKeyboard().IsKeyPressed(K_E))
        {
            auto nextVehicleType = static_cast<VehicleType>((static_cast<int>(currentVehicleType) + 1) %
                                                            static_cast<int>(VehicleType::COUNT));
            _SetVehicle(nextVehicleType);
            _ChangeImageRight();
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

    void PlayerController::_InitializeHUD()
    {
        auto _scene = GetGameObject().GetScene();

        _hudLeft = _scene->CreateGameObject("HUD_Left_Icon");
        _hudMiddle = _scene->CreateGameObject("HUD_Middle_Icon");
        _hudRight = _scene->CreateGameObject("HUD_Right_Icon");

        Viewport viewportLeft;
        viewportLeft.height = 0.06f;
        viewportLeft.width = 0.05f;
        viewportLeft.x = 0.75f;
        viewportLeft.y = 0.075f;

        Viewport viewportRight;
        viewportRight.height = 0.06f;
        viewportRight.width = 0.05f;
        viewportRight.x = 0.9f;
        viewportRight.y = 0.075f;

        Viewport viewportMiddle;
        viewportMiddle.height = 0.1f;
        viewportMiddle.width = 0.1f;
        viewportMiddle.x = 0.8f;
        viewportMiddle.y = 0.05f;

        const std::string INIT_PATH = _imagePaths[0];

        _hudLeft.AddComponent<UIElement>(
            UIElement{ .viewport = viewportLeft,
                       .content = UIImage{ .textureFilepath = INIT_PATH, .filter = Material::FilterMode::POINT } });
        _hudMiddle.AddComponent<UIElement>(
            UIElement{ .viewport = viewportMiddle,
                       .content = UIImage{ .textureFilepath = INIT_PATH, .filter = Material::FilterMode::POINT } });
        _hudRight.AddComponent<UIElement>(
            UIElement{ .viewport = viewportRight,
                       .content = UIImage{ .textureFilepath = INIT_PATH, .filter = Material::FilterMode::POINT } });
    }

    void PlayerController::_ChangeImageRight()
    {
        if (_imagePaths.empty())
            return;

        _currentIndex = (_currentIndex + 1) % _imagePaths.size();

        _UpdateHUDDisplay();
    }

    void PlayerController::_ChangeImageLeft()
    {
        if (_imagePaths.empty())
            return;

        size_t numImages = _imagePaths.size();
        _currentIndex = (_currentIndex - 1 + numImages) % numImages;

        _UpdateHUDDisplay();
    }

    void PlayerController::_UpdateHUDDisplay()
    {
        size_t numImages = _imagePaths.size();

        size_t current = _currentIndex;
        size_t prev = (current - 1 + numImages) % numImages;
        size_t next = (current + 1) % numImages;

        auto updateImage = [](GameObject& hudObject, const std::string& path)
        {
            auto& uiElement = hudObject.GetComponent<UIElement>();
            auto& uiImage = std::get<UIImage>(uiElement.content);
            uiImage.SetTexturePath(path);
            hudObject.SetActive(true);
        };

        updateImage(_hudMiddle, _imagePaths[current]);
        updateImage(_hudLeft, _imagePaths[prev]);
        updateImage(_hudRight, _imagePaths[next]);
    }
} // namespace GameLogic
