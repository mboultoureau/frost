#include "Frost/Scene/Components/Transform.h"
#include "Frost/Utils/Math/Matrix.h"

namespace Frost::Component
{
    Math::Matrix4x4 Transform::GetTransformMatrix() const
    {
        using namespace DirectX;
        using namespace Frost::Math;

        XMMATRIX scaleMat = XMMatrixScalingFromVector(Math::vector_cast<XMVECTOR>(scale));
        XMMATRIX rotMat = XMMatrixRotationQuaternion(Math::vector_cast<XMVECTOR>(rotation));
        XMMATRIX transMat = XMMatrixTranslationFromVector(Math::vector_cast<XMVECTOR>(position));

        XMMATRIX transformMatrix = scaleMat * rotMat * transMat;

        Matrix4x4 result;
        StoreMatrix(&result, transformMatrix);

        return result;
    }
} // namespace Frost::Component