#include "Ogre.h"
#include "../Game.h"

class RotateOgre : public Frost::Script
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

Ogre::Ogre()
{
	Scene& _scene = Game::GetScene();

	_ogre = _scene.CreateGameObject("Ogre");
	_scene.AddComponent<Frost::Transform>(
		_ogre,
		Frost::Transform::Vector3{ 0.0f, 5.0f, 5.0f },
		Frost::Transform::Vector3{ angle_traits<Degree>::to_neutral(-90.0), angle_traits<Degree>::to_neutral(-90.0), 0.0f },
		Frost::Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_ogre, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::ModelRenderer>(_ogre, "./resources/meshes/diffuse.fbx");
	_scene.AddScript<RotateOgre>(_ogre);
}
