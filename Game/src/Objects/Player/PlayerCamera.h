#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include "Frost/Scene/Components/Script.h"
#include "Player.h"
#include <Frost/Scene/Components/Meta.h>
#include <Frost/Renderer/PostEffect/FogEffect.h>
#include <Frost/Renderer/PostEffect/ScreenShakeEffect.h>

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
    void UpdateSpringCamRotation(const JPH::Vec3& pos, float deltaTime);
    void ProcessInput(float deltaTime);

private:
    float _cameraPivotRotationX = 0.0f;
    float _cameraPivotRotationY = 0.0f;
    float _yaw = 0.0f;
    float _pitch = 0.0f;
    bool _freeCam = false;
    const float _mouseSensitivity = 0.003f;
};

class PlayerCamera
{
public:
    friend class Player;

    GameObject& GetCameraId() { return _camera; }
    PlayerCamera(Player* _playerController);

    // PostEffect calls
    void Shake(float duration, float amplitude, ScreenShakeEffect::AttenuationType type);
    void SetRadialBlurStrength(float strength);
    void SetChromaticAberrationStrength(float strength);

private:
    Player* _playerController;
    GameObject _cameraPivot;
    GameObject _3rdPersVirtCamera;
    GameObject _camera;
    RadialBlurEffect* radialBlur = nullptr;
    ScreenShakeEffect* screenShake = nullptr;
    ChromaticAberrationEffect* chromaticAberration = nullptr;

    JPH::BodyID _cameraBodyID;
    JPH::BodyInterface* _bodyInter;

    const float _maxRadialBlurStrength = 0.07f;
    const float _maxShakeAmplitude = 10.0f;
    const float _maxShakeDuration = 2.0f;
};