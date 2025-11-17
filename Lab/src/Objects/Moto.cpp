#include "Moto.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class RotateMoto : public Frost::Script
{
public:
	void OnFixedUpdate(float deltaTime) override
	{
		auto transform = GetECS()->GetComponent<Transform>(GetGameObject());

		const float rotationSpeed = 1.0f;
		float angle = rotationSpeed * deltaTime;
		transform->Rotate(Vector3{ 0.0f, angle, 0.0f });
	}
};

Moto::Moto()
{
	Scene& _scene = Game::GetScene();

	_moto = _scene.CreateGameObject("Moto");
	_scene.AddComponent<Transform>(
		_moto,
		Vector3{ 0.0f, 5.0f, 5.0f },
		EulerAngles{ 0.0f, 0.0f, 0.0_deg },
		Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<WorldTransform>(_moto, Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<StaticMesh>(_moto, "./resources/meshes/moto.glb");
	_scene.AddScript<RotateMoto>(_moto);
}