#pragma once

#include "Frost/Scene/ECS/Component.h"

#include <DirectXMath.h>

namespace Frost
{

	struct Transform : public Component
	{
		using Vector3 = DirectX::XMFLOAT3;
		using Vector4 = DirectX::XMFLOAT4;

		Vector3 position;
		Vector4 rotation;
		Vector3 scale;

		Transform() noexcept
			: position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f)
		{
		}

		Transform(const Vector3& pos, const Vector4& rot, const Vector3& scl) noexcept
			: position(pos), rotation(rot), scale(scl)
		{
		}

		Transform(const Vector3& pos) noexcept
			: position(pos), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f)
		{
		}

		Transform(const Vector3& pos, const Vector3& euler_angles, const Vector3& scl) noexcept
			: position(pos), scale(scl)
		{
			using namespace DirectX;

			XMVECTOR euler = DirectX::XMLoadFloat3(&euler_angles);

			XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(
				euler_angles.x,
				euler_angles.y,
				euler_angles.z
			);

			DirectX::XMStoreFloat4(&rotation, quaternion);
		}

		void Rotate(const Vector3& euler_angles)
		{
			using namespace DirectX;
			XMVECTOR currentRotation = DirectX::XMLoadFloat4(&rotation);
			XMVECTOR deltaRotation = DirectX::XMQuaternionRotationRollPitchYaw(
				euler_angles.x,
				euler_angles.y,
				euler_angles.z
			);
			XMVECTOR newRotation = DirectX::XMQuaternionMultiply(currentRotation, deltaRotation);
			DirectX::XMStoreFloat4(&rotation, newRotation);
		}
	};
}
