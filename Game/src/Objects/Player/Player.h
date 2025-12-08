#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include "../HUD/SelectedImageManager.h"

using namespace Frost::Math;
using namespace Frost;
using namespace Frost::Component;
class PlayerCamera;
class Vehicle;
class Player
{
public:
    enum VehicleType
    {
        BIKE = 0,
        BOAT = 1,
        PLANE = 2
    };

    Player();
    Scene* GetScene() { return _scene; };

    void SetPlayerVehicle(Player::VehicleType type);

    SelectedImageManager* GetImageManager() { return _imageManager; }

    std::pair<VehicleType, Vehicle*> GetCurrentVehicle() { return { _currentVehicleType, _currentVehicle }; };
    bool IsInWater() { return _isInWater; }
    void SetIsInWater(bool b) { _isInWater = b; }

    int GetVehicleNumber() { return _vehicles.size(); };
    GameObject GetPlayerID() { return _playerId; };
    JPH::BodyID GetBodyID() { return _playerId.GetComponent<RigidBody>().runtimeBodyID; };

    Timer transitionTimer;
    GameObject transitionRenderer;

    void SetRespawnPoint(Math::Vector3 lastCheckPointPosition,
                         Math::Vector4 lastRespawnRotation); // TODO : add lastVehiculeRespawnType

    void Warp(Vector3 position, Vector4 rotation,
              Vector3 speed); // TODO: add vehicle type
    void WarpCamera(Vector3 offset, Vector4 rotation, Vector3 speed);
    PlayerCamera* GetCamera() { return _playerCamera; }

    bool forceSpecificCameraPos = false;
    bool allowInput = false;

    static Player* GetClosestPlayer(Vector3 worldPos);
    static void ResetPlayers();

private:
    static std::vector<Player*> _players;

    GameObject _playerId;

    Scene* _scene = nullptr;
    PlayerCamera* _playerCamera = nullptr;

    Math::Vector3 _lastRespawnPosition;
    Math::Vector4 _lastRespawnRotation;

    SelectedImageManager* _imageManager = nullptr;

    Vehicle* _currentVehicle = nullptr;
    Player::VehicleType _currentVehicleType;
    std::unordered_map<Player::VehicleType, Vehicle*> _vehicles;

    void _InitializeVehicles();
    void _SummonVehicleTransition();
    void _SetBodyID(JPH::BodyID bodyId) { _playerId.AddComponent<RigidBody>(bodyId); }
    bool _isInWater = false;

    // void InitializePhysics();
    // void ProcessInput(float deltaTime);
    // void UpdatePhysics(float deltaTime);
};
