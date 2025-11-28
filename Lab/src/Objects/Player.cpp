#include "Player.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Scriptable.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <memory>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class PlayerScript : public Frost::Script
{
    float _forward = 0.0f;
    float _right = 0.0f;
    float _up = 0.0f;
    JPH::BodyID _playerBodyID;
    JPH::BodyInterface* _bodyInter;

    // Mouse look
    float _yaw = 0.0f;
    float _pitch = 0.0f;
    Mouse::MousePosition _lastMousePos;
    bool _firstMouseUpdate = true;
    const float _mouseSensitivity = 0.003f;
    const float _moveSpeed = 10.0f;

public:
    void OnInitialize() override
    {
        _playerBodyID = Game::GetScene().GetComponent<RigidBody>(GetGameObject())->physicBody->bodyId;
        _bodyInter = Physics::Get().body_interface;

        // Hide cursor for mouse look
        Input::GetMouse().HideCursor();
        Input::GetMouse().LockCursor();
        _lastMousePos = Input::GetMouse().GetPosition();
        _firstMouseUpdate = true;
    }

    void OnFixedUpdate(float deltaTime) override
    {
        ProcessInput(deltaTime);
        UpdatePhysics(deltaTime);
    }

private:
    void ProcessInput(float deltaTime)
    {
        auto& scene = Game::GetScene();
        auto* window = Application::GetWindow();
        uint32_t centerX = window->GetWidth() / 2;
        uint32_t centerY = window->GetHeight() / 2;

        if (Input::GetKeyboard().IsKeyPressed(K_F1))
        {
            if (Input::GetMouse().IsCursorVisible())
            {
                Input::GetMouse().HideCursor();
                Input::GetMouse().LockCursor();
                Input::GetMouse().SetPosition({ centerX, centerY });

                _firstMouseUpdate = true;
            }
            else
            {
                Input::GetMouse().ShowCursor();
                Input::GetMouse().UnlockCursor();
            }
        }

        if (!Input::GetMouse().IsCursorVisible())
        {
            auto currentMousePos = Input::GetMouse().GetPosition();

            int deltaX = static_cast<int>(currentMousePos.x) - static_cast<int>(centerX);
            int deltaY = static_cast<int>(currentMousePos.y) - static_cast<int>(centerY);

            if (deltaX != 0 || deltaY != 0)
            {
                _yaw += static_cast<float>(deltaX) * _mouseSensitivity;
                _pitch += static_cast<float>(deltaY) * _mouseSensitivity;

                Angle<Radian> limit = 89.0_deg;
                _pitch = std::clamp(_pitch, -limit.value(), limit.value());

                Input::GetMouse().SetPosition({ centerX, centerY });
            }
        }

        // WASD movement
        _forward = 0.0f;
        _right = 0.0f;
        _up = 0.0f;

        if (Input::GetKeyboard().IsKeyDown(K_UP) || Input::GetKeyboard().IsKeyDown(K_W))
        {
            _forward = 1.0f;
        }
        else if (Input::GetKeyboard().IsKeyDown(K_DOWN) || Input::GetKeyboard().IsKeyDown(K_S))
        {
            _forward = -1.0f;
        }

        if (Input::GetKeyboard().IsKeyDown(K_LEFT) || Input::GetKeyboard().IsKeyDown(K_A))
        {
            _right = -1.0f;
        }
        else if (Input::GetKeyboard().IsKeyDown(K_RIGHT) || Input::GetKeyboard().IsKeyDown(K_D))
        {
            _right = 1.0f;
        }

        // Upward/Downward
        if (Input::GetKeyboard().IsKeyDown(K_SPACE))
        {
            _up = 1.0f;
        }
        else if (Input::GetKeyboard().IsKeyDown(K_SHIFT))
        {
            _up = -1.0f;
        }
    }

    void UpdatePhysics(float deltaTime)
    {
        using namespace JPH;
        using namespace DirectX;

        auto& scene = Game::GetScene();

        JPH::Quat rotation = JPH::Quat::sRotation(Vec3::sAxisY(), _yaw);
        _bodyInter->SetRotation(_playerBodyID, rotation, EActivation::DontActivate);

        // Apply pitch to camera child
        auto children = GetGameObject().GetChildren();
        for (auto& child : children)
        {
            if (auto* cam = scene.GetComponent<Camera>(child))
            {
                auto* cameraTransform = scene.GetComponent<Transform>(child);

                XMVECTOR pitchQuat = XMQuaternionRotationRollPitchYaw(_pitch, 0.0f, 0.0f);
                cameraTransform->rotation = vector_cast<Vector4>(pitchQuat);
                break;
            }
        }

        // Calculate movement direction based on yaw
        XMVECTOR forwardDir = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        XMVECTOR rightDir = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        XMMATRIX rotationMatrix = XMMatrixRotationY(_yaw);
        forwardDir = XMVector3Transform(forwardDir, rotationMatrix);
        rightDir = XMVector3Transform(rightDir, rotationMatrix);

        // Calculate velocity
        XMVECTOR velocity = XMVectorScale(forwardDir, _forward * _moveSpeed);
        velocity = XMVectorAdd(velocity, XMVectorScale(rightDir, _right * _moveSpeed));
        velocity = XMVectorAdd(velocity, XMVectorScale(upDir, _up * _moveSpeed));

        XMFLOAT3 vel;
        XMStoreFloat3(&vel, velocity);

        if (_right != 0.0f || _forward != 0.0f || _up != 0.0f)
        {
            Physics::ActivateBody(_playerBodyID);
        }

        _bodyInter->SetAngularVelocity(_playerBodyID, Vec3(0.0f, 0.0f, 0.0f));
        _bodyInter->SetLinearVelocity(_playerBodyID, Vec3(vel.x, vel.y, vel.z));
    }
};

Player::Player()
{
    Scene& scene = Game::GetScene();

    _player = scene.CreateGameObject("Player");
    scene.AddComponent<Transform>(
        _player, Vector3{ 0.0f, 0.0f, -10.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    scene.AddComponent<WorldTransform>(_player);

    // Camera
    _camera = scene.CreateGameObject("Camera", _player);
    scene.AddComponent<Transform>(_camera, Vector3{ 0.0f, 0.0f, 0.0f });
    scene.AddComponent<WorldTransform>(_camera, Vector3{ 0.0f, 0.0f, 0.0f });
    scene.AddComponent<Camera>(_camera);

    auto cameraComponent = scene.GetComponent<Camera>(_camera);
    cameraComponent->backgroundColor.r = 47.0f / 255.0f;
    cameraComponent->backgroundColor.g = 116.0f / 255.0f;
    cameraComponent->backgroundColor.b = 228.0f / 255.0f;
    cameraComponent->backgroundColor.a = 1.0f;
    // cameraComponent->postEffects.push_back(std::make_shared<Frost::ScreenShakeEffect>());
    // cameraComponent->postEffects.push_back(std::make_shared<Frost::RadialBlurEffect>());
    cameraComponent->postEffects.push_back(std::make_shared<Frost::ChromaticAberrationEffect>());

    InitializePhysics();
}

void
Player::InitializePhysics()
{
    using namespace JPH;

    FT_ENGINE_ASSERT(_player != GameObject::InvalidId, "Player GameObject is invalid");
    Scene& scene = Game::GetScene();

    // Create vehicle body
    RVec3 position(0, 0.0f, -10);
    JPH::ShapeRefC sphereShape = JPH::SphereShapeSettings(1.0f).Create().Get();
    BodyCreationSettings player_body_settings(
        sphereShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
    player_body_settings.mGravityFactor = 0.0f;
    scene.AddComponent<RigidBody>(_player, player_body_settings, _player, EActivation::Activate);

    auto _playerBodyID = scene.GetComponent<RigidBody>(_player)->physicBody->bodyId;
    auto _bodyInter = Physics::Get().body_interface;

    scene.AddScript<PlayerScript>(_player);
}