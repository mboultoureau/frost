#include "Editor/Utils/EditorCameraController.h"
#include "Frost/Utils/Math/Vector.h"

using namespace DirectX;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace Editor
{
    void EditorCameraController::OnUpdate(float deltaTime, Transform& transform, bool isViewportFocused)
    {
        auto& mouse = Input::GetMouse();
        auto& keyboard = Input::GetKeyboard();

        using MouseBtn = Frost::Mouse::MouseBoutton;

        bool rightClick = mouse.IsButtonDown(MouseBtn::Right);
        bool leftClick = mouse.IsButtonDown(MouseBtn::Left);
        bool anyClick = rightClick || leftClick;

        if (isViewportFocused && anyClick && !_isActive)
        {
            _isActive = true;
            _lastMousePos = mouse.GetPosition();
            mouse.HideCursor();
            mouse.LockCursor();
        }
        else if (!anyClick && _isActive)
        {
            _isActive = false;
            mouse.ShowCursor();
            mouse.UnlockCursor();
        }

        if (!_isActive || !isViewportFocused)
            return;

        // Rotation and Movement
        auto currentMousePos = mouse.GetPosition();
        float deltaX = (float)((int)currentMousePos.x - (int)_lastMousePos.x);
        float deltaY = (float)((int)currentMousePos.y - (int)_lastMousePos.y);

        mouse.SetPosition(_lastMousePos);

        float currentSpeed = moveSpeed;
        if (keyboard.IsKeyDown(K_SHIFT))
        {
            currentSpeed *= turboMultiplier;
        }

        XMVECTOR forwardDir, rightDir, upDir;
        _CalculateVectors(forwardDir, rightDir, upDir);

        XMVECTOR position = vector_cast<XMVECTOR>(transform.position);

        if (rightClick)
        {
            // Rotation
            _yaw += deltaX * mouseSensitivity;
            _pitch += deltaY * mouseSensitivity;

            // Mouvement WASD
            if (keyboard.IsKeyDown(K_W))
            {
                position = XMVectorAdd(position, XMVectorScale(forwardDir, currentSpeed * deltaTime));
            }
            if (keyboard.IsKeyDown(K_S))
            {
                position = XMVectorSubtract(position, XMVectorScale(forwardDir, currentSpeed * deltaTime));
            }
            if (keyboard.IsKeyDown(K_D))
            {
                position = XMVectorAdd(position, XMVectorScale(rightDir, currentSpeed * deltaTime));
            }
            if (keyboard.IsKeyDown(K_A))
            {
                position = XMVectorSubtract(position, XMVectorScale(rightDir, currentSpeed * deltaTime));
            }

            // Up/Down (Q/E)
            XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            if (keyboard.IsKeyDown(K_E))
            {
                position = XMVectorAdd(position, XMVectorScale(worldUp, currentSpeed * deltaTime));
            }
            if (keyboard.IsKeyDown(K_Q))
            {
                position = XMVectorSubtract(position, XMVectorScale(worldUp, currentSpeed * deltaTime));
            }
        }
        else if (leftClick)
        {
            _yaw += deltaX * mouseSensitivity;

            float moveAmount = -deltaY * mouseSensitivity * currentSpeed * 20.0f;
            position = XMVectorAdd(position, XMVectorScale(forwardDir, moveAmount * deltaTime));
        }

        _pitch = std::clamp(_pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

        XMVECTOR rotationQuat = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, 0.0f);

        transform.rotation = vector_cast<Math::Vector4>(rotationQuat);
        transform.position = vector_cast<Math::Vector3>(position);
    }

    void EditorCameraController::Initialize(const Transform& transform)
    {
        auto euler = transform.GetEulerAngles();
        _pitch = euler.Pitch.value();
        _yaw = euler.Yaw.value();
    }

    void EditorCameraController::_CalculateVectors(XMVECTOR& forward, XMVECTOR& right, XMVECTOR& up)
    {
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(_pitch, _yaw, 0.0f);

        forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);
        right = XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotationMatrix);
        up = XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationMatrix);
    }

} // namespace Editor
