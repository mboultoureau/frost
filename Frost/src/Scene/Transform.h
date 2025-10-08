#pragma once

#include <DirectXMath.h>

namespace Frost
{
	using namespace DirectX;

	class Transform
	{
	public:
		using Vector3 = XMFLOAT3;
		using Radians = float;

		Vector3& GetPosition() { return _position; }
		Vector3& GetRotation() { return _rotation; }
		Vector3& GetScale() { return _scale; }

		void SetPosition(const Vector3& position) { _position = position; }
		void SetRotation(const Vector3& rotation) { _rotation = rotation; }
		void SetScale(const Vector3& scale) { _scale = scale; }

		void Translate(const Vector3& delta);
		void Rotate(const Vector3& delta);
		void Scale(const Vector3& delta);

	private:
		Vector3 _position;
		Vector3 _rotation;
		Vector3 _scale;
	};
}
