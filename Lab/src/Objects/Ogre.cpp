#include "Ogre.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class RotateOgre : public Frost::Script
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

Ogre::Ogre()
{
	Scene& _scene = Game::GetScene();

	_ogre = _scene.CreateGameObject("Ogre");
	_scene.AddComponent<Transform>(
		_ogre,
		Vector3{ 0.0f, 0.0f, 5.0f },
		EulerAngles{ 0.0_deg, 0.0_deg, 0.0f },
		Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<WorldTransform>(_ogre, Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<StaticMesh>(_ogre, "./resources/meshes/diffuse.fbx");
	_scene.AddScript<RotateOgre>(_ogre);
}
