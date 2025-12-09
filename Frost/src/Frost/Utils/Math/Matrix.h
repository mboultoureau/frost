#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Utils/Math/Vector.h"

#include <array>

namespace Frost::Math
{
    struct FROST_API Matrix4x4
    {
        std::array<float, 16> elements;

        Matrix4x4();
        Matrix4x4(const std::array<float, 16>& elements);

        static Matrix4x4 CreateIdentity();
        static Matrix4x4 CreateTranspose(const Matrix4x4& matrix);
        static Matrix4x4 Invert(const Matrix4x4& matrix);
        static Matrix4x4 CreateTranslation(const Vector3& position);
        static Matrix4x4 CreateScale(const Vector3& scale);
        static Matrix4x4 CreateFromQuaternion(const Vector4& quaternion);

        static Matrix4x4 CreateLookToLH(const Vector3& eyePosition,
                                        const Vector3& eyeDirection,
                                        const Vector3& upDirection);
        static Matrix4x4 CreatePerspectiveFovLH(float fov, float aspectRatio, float nearPlane, float farPlane);
        static Matrix4x4 CreateOrthographicLH(float width, float height, float nearPlane, float farPlane);
    };

    inline DirectX::XMMATRIX LoadMatrix(const Matrix4x4& matrix)
    {
        return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(matrix.elements.data()));
    }

    inline void StoreMatrix(Matrix4x4* outMatrix, const DirectX::XMMATRIX& matrix)
    {
        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(outMatrix->elements.data()), matrix);
    }

    FROST_API Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b);

    inline Vector3 TransformCoord(const Vector3& v, const Matrix4x4& m)
    {
        DirectX::XMVECTOR vec = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&v);
        DirectX::XMMATRIX mat = LoadMatrix(m);

        Vector3 result;
        DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&result, DirectX::XMVector3TransformCoord(vec, mat));
        return result;
    }

    inline Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m)
    {
        DirectX::XMVECTOR vec = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&v);
        DirectX::XMMATRIX mat = LoadMatrix(m);

        Vector3 result;
        DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&result, DirectX::XMVector3TransformNormal(vec, mat));
        return result;
    }

    inline Vector4 TransformVector4(const Vector4& v, const Matrix4x4& m)
    {
        DirectX::XMVECTOR vec = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&v));
        DirectX::XMMATRIX mat = LoadMatrix(m);

        Vector4 result;
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result), DirectX::XMVector4Transform(vec, mat));
        return result;
    }
} // namespace Frost::Math