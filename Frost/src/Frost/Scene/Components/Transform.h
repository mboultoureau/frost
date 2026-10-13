#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Vector.h"

#include <DirectXMath.h>

namespace Frost::Component
{
    struct FROST_API Transform : public Component
    {
        Math::Vector3 position;
        Math::Vector4 rotation;
        Math::Vector3 scale;
        bool isDirty = true;

        Transform() noexcept :
            position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f), isDirty(true)
        {
        }

        Transform(const Math::Vector3& pos, const Math::Vector4& rot, const Math::Vector3& scl) noexcept :
            position(pos), rotation(rot), scale(scl), isDirty(true)
        {
        }

        Transform(const Math::Vector3& pos) noexcept :
            position(pos), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f), isDirty(true)
        {
        }

        Transform(const Math::Vector3& pos, const Math::EulerAngles& angles, const Math::Vector3& scl) noexcept :
            position(pos), scale(scl), isDirty(true)
        {
            SetRotation(angles);
        }

        Transform& operator=(const Transform& other) noexcept
        {
            if (this != &other)
            {
                position = other.position;
                rotation = other.rotation;
                scale = other.scale;
                isDirty = true;
            }
            return *this;
        }

        void SetPosition(const Math::Vector3& pos)
        {
            position = pos;
            isDirty = true;
        }

        void SetRotation(const Math::Vector4& rot)
        {
            rotation = rot;
            isDirty = true;
        }

        void SetScale(const Math::Vector3& scl)
        {
            scale = scl;
            isDirty = true;
        }

        void SetDirty(bool dirty = true) { isDirty = dirty; }
        bool IsDirty() const { return isDirty; }

        void SetRotation(const Math::EulerAngles& eulerAngles)
        {
            using namespace DirectX;
            XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(
                eulerAngles.Pitch.value(), eulerAngles.Yaw.value(), eulerAngles.Roll.value());
            rotation = Math::vector_cast<Math::Vector4>(quaternion);
            isDirty = true;
        }

        void Rotate(const Math::EulerAngles& eulerAngles)
        {
            using namespace DirectX;
            XMVECTOR currentRotation = Math::vector_cast<XMVECTOR>(rotation);
            XMVECTOR deltaRotation = XMQuaternionRotationRollPitchYaw(
                eulerAngles.Pitch.value(), eulerAngles.Yaw.value(), eulerAngles.Roll.value());

            XMVECTOR newRotation = XMQuaternionMultiply(deltaRotation, currentRotation);
            rotation = Math::vector_cast<Math::Vector4>(newRotation);
            isDirty = true;
        }

        Math::EulerAngles GetEulerAngles() const { return Math::QuaternionToEulerAngles(rotation); }
        Math::Matrix4x4 GetTransformMatrix() const;

        Math::Vector3 GetForward() const
        {
            using namespace DirectX;
            XMVECTOR forwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
            XMVECTOR currentRotation = Math::vector_cast<XMVECTOR>(rotation);
            XMVECTOR rotatedVector = XMVector3Rotate(forwardVector, currentRotation);
            return Math::vector_cast<Math::Vector3>(rotatedVector);
        }

        Math::Vector3 GetUp() const
        {
            using namespace DirectX;
            XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            XMVECTOR currentRotation = Math::vector_cast<XMVECTOR>(rotation);
            XMVECTOR rotatedVector = XMVector3Rotate(upVector, currentRotation);
            return Math::vector_cast<Math::Vector3>(rotatedVector);
        }

        Math::Vector3 GetRight() const
        {
            using namespace DirectX;
            XMVECTOR rightVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
            XMVECTOR currentRotation = Math::vector_cast<XMVECTOR>(rotation);
            XMVECTOR rotatedVector = XMVector3Rotate(rightVector, currentRotation);
            return Math::vector_cast<Math::Vector3>(rotatedVector);
        }

        void LookAt(const Math::Vector3& target)
        {
            using namespace DirectX;

            XMVECTOR eyePosition = Math::vector_cast<XMVECTOR>(position);
            XMVECTOR focusPosition = Math::vector_cast<XMVECTOR>(target);

            if (XMVector3IsInfinite(eyePosition) || XMVector3IsNaN(eyePosition) || XMVector3IsInfinite(focusPosition) ||
                XMVector3IsNaN(focusPosition))
            {
                return;
            }

            XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            XMVECTOR direction = XMVectorSubtract(focusPosition, eyePosition);
            XMVECTOR lengthSq = XMVector3LengthSq(direction);

            if (XMVectorGetX(lengthSq) < 0.00001f)
            {
                return;
            }

            XMVECTOR dirNormalized = XMVector3Normalize(direction);
            XMVECTOR dot = XMVector3Dot(dirNormalized, upDirection);
            float dotVal = fabsf(XMVectorGetX(dot));

            if (dotVal > 0.99f)
            {
                upDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
            }

            XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
            XMVECTOR determinant;
            XMMATRIX worldMatrix = XMMatrixInverse(&determinant, viewMatrix);

            XMVECTOR quaternion = XMQuaternionRotationMatrix(worldMatrix);
            rotation = Math::vector_cast<Math::Vector4>(quaternion);
            isDirty = true;
        }
    };
} // namespace Frost::Component
