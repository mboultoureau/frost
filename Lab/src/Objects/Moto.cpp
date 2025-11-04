#include "Moto.h"
#include "../Game.h"

class RotateMoto : public Frost::Script
{
public:
	void OnFixedUpdate(float deltaTime) override
	{
		auto transform = GetECS()->GetComponent<Frost::Transform>(GetGameObject());

		const float rotationSpeed = 1.0f;
		float angle = rotationSpeed * deltaTime;
		transform->Rotate(Frost::Transform::Vector3{ 0.0f, angle, 0.0f });
	}
};

Moto::Moto()
{
	Scene& _scene = Game::GetScene();

	_moto = _scene.CreateGameObject("Moto");
	_scene.AddComponent<Frost::Transform>(
		_moto,
		Frost::Transform::Vector3{ 0.0f, 5.0f, 5.0f },
		Frost::Transform::Vector3{ angle_traits<Degree>::to_neutral(-90.0), angle_traits<Degree>::to_neutral(-90.0), 0.0f },
		Frost::Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_moto, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::ModelRenderer>(_moto, "./resources/meshes/moto.glb");
	_scene.AddScript<RotateMoto>(_moto);
}
