#include "Transform.h"

using namespace Frost;

void Transform::Translate(const Vector3& delta)
{
	SetPosition(Vector3{
		GetPosition().x + delta.x,
		GetPosition().y + delta.y,
		GetPosition().z + delta.z
	});
};

void Transform::Rotate(const Vector3& delta)
{
    SetRotation(Vector3{
		GetRotation().x + delta.x,
		GetRotation().y + delta.y,
		GetRotation().z + delta.z
	});
};

void Transform::Scale(const Vector3& delta)
{
	SetScale(Vector3{
		GetScale().x * delta.x,
		GetScale().y * delta.y,
		GetScale().z * delta.z
	});
};
