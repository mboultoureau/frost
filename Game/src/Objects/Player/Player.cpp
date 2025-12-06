#include "Player.h"
#include "../../Game.h"
#include "../../Physics/PhysicsLayer.h"

#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include "Frost/Scene/Components/Meta.h"
#include "PlayerCamera.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

#include "../CheckPoint.h"
#include "PlayerScript.h"
#include "../HUD/Speedometer.h"
#include "../HUD/SelectedImageManager.h"
#include "Vehicles/Bike.h"
#include "Vehicles/Boat.h"
#include "Vehicles/Plane.h"
#include "Vehicles/Vehicle.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Player::Player()
{
    _players.push_back(this);
    _scene = &Game::GetScene();

    // Create Player Game Object -------------
    _playerId = _scene->CreateGameObject("Player");
    _playerId.AddComponent<Transform>(
        Vector3{ -365.0f, 80.0f, -100.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _playerId.AddComponent<WorldTransform>();

    // Create TransitionModelRenderer -----------
    transitionRenderer = _scene->CreateGameObject("Transition Model renderer", _playerId);
    transitionRenderer.AddComponent<Transform>(
        Vector3{ 0.0f, 0, 0.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 2.0f, 2.0f, 2.0f });
    transitionRenderer.AddComponent<WorldTransform>();
    transitionRenderer.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/sphere.fbx" });

    // Create Selection Images
    _imageManager = new Frost::SelectedImageManager();

    // Create Vehicules structures -------------
    _InitializeVehicles();
    SetPlayerVehicle(VehicleType::BIKE);
    _playerId.AddScript<PlayerScript>(this);

    // Create Player Speedometer HUD
    _playerId.AddScript<SpeedometerScript>(*this);

    // Create playerCameras Game Objects -------------
    _playerCamera = new PlayerCamera(this);
}

void
Player::_InitializeVehicles()
{
    auto pos = Vector3{ 0.0f, 0.0f, 0.0f };
    // Bike
    auto bike = new Bike(this,
                         Vehicle::RendererParameters("Moto Renderer",
                                                     "./resources/meshes/moto.glb",
                                                     Vector3{ 0, -0.4f, 0 },
                                                     EulerAngles{ 0.0, 180.0_deg, 0.0_deg },
                                                     Vector3{ .6f, .6f, .6f }));
    _vehicles.insert({ VehicleType::BIKE, bike });

    // Boat
    auto boat = new Boat(this,
                         Vehicle::RendererParameters("Boat Renderer",
                                                     "./resources/meshes/boat.glb",
                                                     Vector3(3.f, -0.2, 0),
                                                     EulerAngles(0.0, 90.0_deg, 0.0_deg),
                                                     Vector3{ .01f, .015f, .015f }));
    _vehicles.insert({ VehicleType::BOAT, boat });

    // Plane
    auto plane = new Plane(this,
                           Vehicle::RendererParameters("Plane Renderer",
                                                       "./resources/meshes/StarSparrow01.fbx",
                                                       Vector3{ 0, -0.4f, 0 },
                                                       EulerAngles{ 0.0_deg, 180.0_deg, 0.0f },
                                                       Vector3{ .003f, .003f, .003f }));
    _vehicles.insert({ VehicleType::PLANE, plane });
}

void
Player::SetPlayerVehicle(Player::VehicleType type)
{
    using namespace JPH;
    Vec3 linearSpeed = Vec3();
    Vec3 angularSpeed = Vec3();

    if (_currentVehicle)
    {
        Physics::Get().body_interface->GetLinearAndAngularVelocity(
            _playerId.GetComponent<RigidBody>().runtimeBodyID, linearSpeed, angularSpeed);
        _currentVehicle->Disappear();
    }
    _currentVehicle = _vehicles[type];
    _currentVehicleType = type;
    _SummonVehicleTransition();
    auto bodyId = _currentVehicle->Appear();
    if (_playerId.HasComponent<RigidBody>())
    {
        _SetBodyID(bodyId);
        Physics::Get().body_interface->SetLinearAndAngularVelocity(
            _playerId.GetComponent<RigidBody>().runtimeBodyID, linearSpeed, angularSpeed);
    }
    else
    {
        _playerId.AddComponent<RigidBody>(_currentVehicle->GetBodyID());
        Physics::Get().body_interface->SetLinearAndAngularVelocity(
            _playerId.GetComponent<RigidBody>().runtimeBodyID, linearSpeed, angularSpeed);
    }
}
void
Player::_SummonVehicleTransition()
{
    FT_INFO("Vehicle changed");
    transitionRenderer.SetActive(true);
    transitionTimer.Resume();
    transitionTimer.Start();
}

void
Player::SetRespawnPoint(Math::Vector3 lastCheckPointPosition,
                        Math::Vector4 lastRespawnRotation) // TODO : add lastVehiculeRespawnType
{
    _lastRespawnPosition = lastCheckPointPosition;
    _lastRespawnRotation = lastRespawnRotation;
    // TODO : add lastVehiculeRespawnType set
}

void
Player::Warp(Vector3 position,
             Vector4 rotation,
             Vector3 speed) // TODO: add vehicle type
{
    auto& bodyInter = Physics::GetBodyInterface();
    Scene& scene = Game::GetScene();
    auto playerTransform = scene.GetComponent<WorldTransform>(_playerId);

    playerTransform->position = position;
    auto playerBodyId = _scene->GetComponent<RigidBody>(_playerId)->runtimeBodyID;
    bodyInter.SetPosition(playerBodyId, Math::vector_cast<JPH::Vec3>(position), JPH::EActivation::Activate);
    bodyInter.SetLinearVelocity(playerBodyId, Math::vector_cast<JPH::Vec3>(speed));
    bodyInter.SetRotation(playerBodyId, { rotation.x, rotation.y, rotation.z, rotation.w }, JPH::EActivation::Activate);
}

void
Player::WarpCamera(Vector3 offset, Vector4 rotation, Vector3 speed)
{
    auto& bodyInter = Physics::GetBodyInterface();
    Scene& scene = Game::GetScene();
    auto playerTransform = scene.GetComponent<WorldTransform>(_playerId);

    Vector3 forcedNewCameraPos = playerTransform->position + offset;

    auto camBodyId = _scene->GetComponent<RigidBody>(_playerCamera->_camera)->runtimeBodyID;

    bodyInter.SetPosition(
        camBodyId, { forcedNewCameraPos.x, forcedNewCameraPos.y, forcedNewCameraPos.z }, JPH::EActivation::Activate);
    bodyInter.SetLinearVelocity(camBodyId, { speed.x, speed.y, speed.z });
    bodyInter.SetRotation(camBodyId, { rotation.x, rotation.y, rotation.z, rotation.w }, JPH::EActivation::Activate);

    forceSpecificCameraPos = true;
}

std::vector<Player*> Player::_players = {};
Player*
Player::GetClosestPlayer(Vector3 worldPos)
{
    if (_players.size() == 0)
        return nullptr;

    auto closestPlayer = _players[0];
    auto playerPos = closestPlayer->GetPlayerID().GetComponent<WorldTransform>().position;
    float distSq = (worldPos.x - playerPos.x) * (worldPos.x - playerPos.x) +
                   (worldPos.y - playerPos.y) * (worldPos.y - playerPos.y) +
                   (worldPos.z - playerPos.z) * (worldPos.z - playerPos.z);
    for (auto p : _players)
    {
        auto newPos = p->GetPlayerID().GetComponent<WorldTransform>().position;
        float newDistSq = (worldPos.x - newPos.x) * (worldPos.x - newPos.x) +
                          (worldPos.y - newPos.y) * (worldPos.y - newPos.y) +
                          (worldPos.z - newPos.z) * (worldPos.z - newPos.z);
        if (newDistSq < distSq)
        {
            closestPlayer = p;
            distSq = newDistSq;
        }
    }
    return closestPlayer;
}

void
Player::ResetPlayers()
{
    for (auto p : _players)
    {
        p->GetCurrentVehicle().second->Disappear();
    }
    _players.clear();
}
