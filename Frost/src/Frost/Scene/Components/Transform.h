#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Vector.h"

#include <DirectXMath.h>

namespace Frost::Component
{

    struct Transform : public Component
    {
        Math::Vector3 position;
        Math::Vector4 rotation;
        Math::Vector3 scale;

        Transform() noexcept : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f) {}

        Transform(const Math::Vector3& pos, const Math::Vector4& rot, const Math::Vector3& scl) noexcept :
            position(pos), rotation(rot), scale(scl)
        {
        }

        Transform(const Math::Vector3& pos) noexcept :
            position(pos), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f)
        {
        }

        Transform(const Math::Vector3& pos, const Math::EulerAngles& angles, const Math::Vector3& scl) noexcept :
            position(pos), scale(scl)
        {
            SetRotation(angles);
        }

        void SetRotation(const Math::EulerAngles& eulerAngles)
        {
            using namespace DirectX;
            XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(
                eulerAngles.Pitch.value(), eulerAngles.Yaw.value(), eulerAngles.Roll.value());
            rotation = Math::vector_cast<Math::Vector4>(quaternion);
        }

        void Rotate(const Math::EulerAngles& eulerAngles)
        {
            using namespace DirectX;
            XMVECTOR currentRotation = Math::vector_cast<XMVECTOR>(rotation);
            XMVECTOR deltaRotation = XMQuaternionRotationRollPitchYaw(
                eulerAngles.Pitch.value(), eulerAngles.Yaw.value(), eulerAngles.Roll.value());

            XMVECTOR newRotation = XMQuaternionMultiply(currentRotation, deltaRotation);
            rotation = Math::vector_cast<Math::Vector4>(newRotation);
        }

        Math::EulerAngles GetEulerAngles() const { return Math::QuaternionToEulerAngles(rotation); }

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
    };
} // namespace Frost::Component
