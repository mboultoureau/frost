#include "Matrix.h"
#include <DirectXMath.h>

namespace Frost::Math
{


    Matrix4x4::Matrix4x4()
    {
        elements = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    Matrix4x4::Matrix4x4(const std::array<float, 16>& elements)
    {
		this->elements = elements;
    }

    Matrix4x4 Matrix4x4::CreateIdentity()
    {
        return Matrix4x4();
    }

    Matrix4x4 Matrix4x4::CreateTranspose(const Matrix4x4& matrix)
    {
        Matrix4x4 result;
        DirectX::XMMATRIX m = LoadMatrix(matrix);
        StoreMatrix(&result, DirectX::XMMatrixTranspose(m));
        return result;
    }

    Matrix4x4 Matrix4x4::CreateTranslation(const Vector3& position)
    {
        Matrix4x4 result;
        StoreMatrix(&result, DirectX::XMMatrixTranslation(position.x, position.y, position.z));
        return result;
    }

    Matrix4x4 Matrix4x4::CreateScale(const Vector3& scale)
    {
        Matrix4x4 result;
        StoreMatrix(&result, DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
        return result;
    }

    Matrix4x4 Matrix4x4::CreateFromQuaternion(const Vector4& quaternion)
    {
        Matrix4x4 result;
        DirectX::XMVECTOR q = vector_cast<DirectX::XMVECTOR>(quaternion);
        StoreMatrix(&result, DirectX::XMMatrixRotationQuaternion(q));
        return result;
    }

    Matrix4x4 Matrix4x4::CreateLookToLH(const Vector3& eyePosition, const Vector3& eyeDirection, const Vector3& upDirection)
    {
        Matrix4x4 result;
        DirectX::XMVECTOR eyePosVec = vector_cast<DirectX::XMVECTOR>(eyePosition);
        DirectX::XMVECTOR eyeDirVec = vector_cast<DirectX::XMVECTOR>(eyeDirection);
        DirectX::XMVECTOR upDirVec = vector_cast<DirectX::XMVECTOR>(upDirection);
        StoreMatrix(&result, DirectX::XMMatrixLookToLH(eyePosVec, eyeDirVec, upDirVec));
        return result;
    }

    Matrix4x4 Matrix4x4::CreatePerspectiveFovLH(float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        Matrix4x4 result;
        StoreMatrix(&result, DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));
        return result;
    }

    Matrix4x4 Matrix4x4::CreateOrthographicLH(float width, float height, float nearPlane, float farPlane)
    {
        Matrix4x4 result;
        StoreMatrix(&result, DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane));
        return result;
    }

    Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b)
    {
        Matrix4x4 result;
        DirectX::XMMATRIX matA = LoadMatrix(a);
        DirectX::XMMATRIX matB = LoadMatrix(b);
        StoreMatrix(&result, DirectX::XMMatrixMultiply(matA, matB));
        return result;
    }

}