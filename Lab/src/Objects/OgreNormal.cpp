#include "OgreNormal.h"
#include "../Game.h"

class RotateOgreNormal : public Frost::Script
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

OgreNormal::OgreNormal()
{
	Scene& _scene = Game::GetScene();

	_ogreNormal = _scene.CreateGameObject("Ogre Normal");
	_scene.AddComponent<Frost::Transform>(
		_ogreNormal,
		Frost::Transform::Vector3{ 0.0f, 5.0f, 5.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_ogreNormal, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::ModelRenderer>(_ogreNormal, "./resources/meshes/normal.fbx");
	_scene.AddScript<RotateOgreNormal>(_ogreNormal);
}
