#pragma once

#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Renderer/BoundingBox.h"
#include "Matrix.h"

#undef min
#undef max

namespace Frost::Math
{
    inline Matrix4x4 GetTransformMatrix(const Component::WorldTransform& transform)
    {
        Matrix4x4 scaleMat = Matrix4x4::CreateScale(transform.scale);
        Matrix4x4 rotationMat = Matrix4x4::CreateFromQuaternion(transform.rotation);
        Matrix4x4 translationMat = Matrix4x4::CreateTranslation(transform.position);

        return scaleMat * rotationMat * translationMat;
    }

    inline Matrix4x4 GetProjectionMatrix(const Component::Camera& camera, float aspectRatio)
    {
        if (camera.projectionType == Component::Camera::ProjectionType::Orthographic)
        {
            float orthoHeight = camera.orthographicSize;
            float orthoWidth = orthoHeight * aspectRatio;
            return Matrix4x4::CreateOrthographicLH(orthoWidth, orthoHeight, camera.nearClip, camera.farClip);
        }
        else
        {
            float fov = Angle<Radian>(camera.perspectiveFOV).value();
            return Matrix4x4::CreatePerspectiveFovLH(fov, aspectRatio, camera.nearClip, camera.farClip);
        }
    }

    inline Matrix4x4 GetViewMatrix(const Component::Transform& cameraTransform)
    {
        DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR worldForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        DirectX::XMFLOAT3 posFloat3 = vector_cast<DirectX::XMFLOAT3>(cameraTransform.position);
        DirectX::XMFLOAT4 rotFloat4 = vector_cast<DirectX::XMFLOAT4>(cameraTransform.rotation);

        DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat3(&posFloat3);
        DirectX::XMVECTOR cameraRotationQuat = DirectX::XMLoadFloat4(&rotFloat4);

        DirectX::XMVECTOR cameraUp = DirectX::XMVector3Rotate(worldUp, cameraRotationQuat);
        DirectX::XMVECTOR cameraDirection = DirectX::XMVector3Rotate(worldForward, cameraRotationQuat);

        Matrix4x4 viewMatrix = Matrix4x4::CreateLookToLH(
            vector_cast<Vector3>(positionVec), vector_cast<Vector3>(cameraDirection), vector_cast<Vector3>(cameraUp));

        return viewMatrix;
    }

    inline BoundingBox TransformBoundingBox(const BoundingBox& localBounds, const Matrix4x4& transform)
    {
        Vector3 corners[8] = { { localBounds.min.x, localBounds.min.y, localBounds.min.z },
                               { localBounds.max.x, localBounds.min.y, localBounds.min.z },
                               { localBounds.min.x, localBounds.max.y, localBounds.min.z },
                               { localBounds.min.x, localBounds.min.y, localBounds.max.z },
                               { localBounds.max.x, localBounds.max.y, localBounds.min.z },
                               { localBounds.min.x, localBounds.max.y, localBounds.max.z },
                               { localBounds.max.x, localBounds.min.y, localBounds.max.z },
                               { localBounds.max.x, localBounds.max.y, localBounds.max.z } };

        Vector4 transformedCorner = TransformVector4({ corners[0].x, corners[0].y, corners[0].z, 1.0f }, transform);
        Vector3 newMin = { transformedCorner.x, transformedCorner.y, transformedCorner.z };
        Vector3 newMax = newMin;

        for (int i = 1; i < 8; ++i)
        {
            transformedCorner = TransformVector4({ corners[i].x, corners[i].y, corners[i].z, 1.0f }, transform);

            newMin.x = std::min(newMin.x, transformedCorner.x);
            newMin.y = std::min(newMin.y, transformedCorner.y);
            newMin.z = std::min(newMin.z, transformedCorner.z);

            newMax.x = std::max(newMax.x, transformedCorner.x);
            newMax.y = std::max(newMax.y, transformedCorner.y);
            newMax.z = std::max(newMax.z, transformedCorner.z);
        }

        return BoundingBox(newMin, newMax);
    }
} // namespace Frost::Math