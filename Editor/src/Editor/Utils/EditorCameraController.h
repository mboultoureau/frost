#pragma once

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Input/Input.h"
#include "Frost/Utils/Math/Angle.h"

#include <DirectXMath.h>
#include <algorithm>

namespace Editor
{
    class EditorCameraController
    {
        using MousePos = Frost::Mouse::MousePosition;
        using Transform = Frost::Component::Transform;
        using XMVECTOR = DirectX::XMVECTOR;

    public:
        float moveSpeed = 10.0f;
        float turboMultiplier = 3.0f;
        float mouseSensitivity = 0.003f;
        float damping = 5.0f;

    private:
        float _yaw = 0.0f;
        float _pitch = 0.0f;

        MousePos _lastMousePos;
        bool _isActive = false;

    public:
        void OnUpdate(float deltaTime, Transform& transform, bool isViewportFocused);
        void Initialize(const Transform& transform);

    private:
        void _CalculateVectors(XMVECTOR& forward, XMVECTOR& right, XMVECTOR& up);
    };
} // namespace Editor