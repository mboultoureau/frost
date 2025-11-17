#pragma once

#include "Matrix.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Utils/Math/Angle.h"

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
            return Matrix4x4::CreatePerspectiveFovLH(
                fov,
                aspectRatio,
                camera.nearClip,
                camera.farClip
            );
        }
    }

    inline Matrix4x4 GetViewMatrix(const Component::WorldTransform& cameraTransform)
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
            vector_cast<Vector3>(positionVec),
            vector_cast<Vector3>(cameraDirection),
            vector_cast<Vector3>(cameraUp)
        );

        return viewMatrix;
    }
}