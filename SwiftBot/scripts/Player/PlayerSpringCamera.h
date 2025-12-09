#pragma once

#include "Frost.h"

namespace GameLogic
{
    class PlayerSpringCamera : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnFixedUpdate(float fixedDeltaTime) override;

    private:
        void _ProcessInput(float fixedDeltaTime);
        void _UpdateTPCam(float fixedDeltaTime);
        void _UpdateSpringCam(float fixedDeltaTime);
        void _UpdateSpringCamRotation(const JPH::Vec3& pos, float fixedDeltaTime);
        JPH::Quat _LookAtQuaternion(const JPH::Vec3& cameraPos, const JPH::Vec3& targetPos);

    private:
        Frost::GameObject _player;
        Frost::GameObject _playerController;
        Frost::GameObject _camera;
        Frost::GameObject _cameraPivot;
        Frost::GameObject _desiredCameraLocation;

        Frost::GameObject _moto;
        Frost::GameObject _boat;
        Frost::GameObject _plane;

        Frost::ScreenShakeEffect* _screenShake{ nullptr };
        Frost::RadialBlurEffect* _radialBlur{ nullptr };

        bool _freeCam = false;
        bool isThirdPerson = true;

        float _yaw = 0.0f;
        float _pitch = 0.0f;
        float _cameraPivotRotationX = 0.0f;
        float _cameraPivotRotationY = 0.0f;

        const float _mouseSensitivity = 0.003f;

        float _stiffness = 15.0f;
        float _playerCullingDistance = 5.0f;
        float _defaultThirdPersonCameraDistance = -10.0f;
        float _thirdPersonCamVelocityToDistance = -0.0005f;
    };
} // namespace GameLogic