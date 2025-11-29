#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include "Frost/Scene/Components/Script.h"
#include "Player.h"
#include <Frost/Scene/Components/Meta.h>
#include <Frost/Renderer/PostEffect/FogEffect.h>

using namespace Frost;

class Player;

class PlayerSpringCameraScript : public Script
{
public:
    PlayerSpringCameraScript(GameObject cameraPivot,
                             GameObject thirdPersonCamera,
                             GameObject springCam,
                             Player* player) :
        thirdPersonCamera{ thirdPersonCamera },
        springCam{ springCam },
        cameraPivot{ cameraPivot },
        playerManager{ player },
        player{ player->GetPlayerID() },
        scene{ player->GetScene() }
    {
        springCam.GetComponent<Camera>().GetEffect<FogEffect>()->SetFog(skyFogMinDepth, skyFogStrength, skyFogColor);
    };

    // References
    Player* playerManager;
    Scene* scene;
    GameObject thirdPersonCamera;
    GameObject player;
    GameObject springCam;
    GameObject cameraPivot;
    bool isThirdPerson = true;

    // Spring parameters
    float stiffness = 15.0f;
    float defaultThirdPersonCameraDistance = -10.0f;
    float playerCullingDistance = 5.0f;
    float thirdPersonCamVelocityToDistance = -0.0005f;

    // Fog parameters
    float waterFogMinDepth = 0.85f;
    float waterFogStrength = 0.85f;
    Vector3 waterFogColor = { 0.f, 0.3f, 0.8f };
    float skyFogMinDepth = 0.9975f;
    float skyFogStrength = .85f;
    Vector3 skyFogColor = { 0.7f, 0.7f, 0.8f };

    // Interfaces
    void OnFixedUpdate(float deltaTime) override;
    void OnCollisionStay(BodyOnContactParameters params, float deltaTime) override;
    void OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime);

    // Functions
    void UpdateTPCam(float deltaTime);
    JPH::Quat LookAtQuaternion(const JPH::Vec3& cameraPos, const JPH::Vec3& targetPos);
    void UpdateSpringCam(float deltaTime);
    void ProcessInput(float deltaTime);

private:
    float _cameraPivotRotationX = 0.0f;
    float _cameraPivotRotationY = 0.0f;
};

class PlayerCamera
{
public:
    friend class Player;

    GameObject& GetCameraId() { return _camera; }
    PlayerCamera(Player* _player);

private:
    Player* _player;
    GameObject _cameraPivot;
    GameObject _3rdPersVirtCamera;
    GameObject _camera;

    JPH::BodyID _cameraBodyID;
    JPH::BodyInterface* _bodyInter;
};