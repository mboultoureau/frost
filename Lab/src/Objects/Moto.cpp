#include "Moto.h"
#include "../Game.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Script.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Utils/Math/Angle.h"

using namespace Frost;
using namespace Frost::Math;

class RotateMoto : public Script
{
public:
	void OnUpdate(float deltaTime) override
	{
		auto& transform = GetGameObject().GetComponent<Component::Transform>();
		transform.Rotate({ 0.0f, 1.0f * deltaTime, 0.0f });
	}
};

Moto::Moto()
{
	Scene& _scene = Game::GetScene();

	_moto = _scene.CreateGameObject("Moto");
	_scene.AddComponent<Component::Transform>(
		_moto,
		Math::Vector3{ 0.0f, 5.0f, 5.0f },
		Math::EulerAngles{ 0.0f, 0.0f, 0.0_deg },
		Math::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Component::WorldTransform>(_moto, Math::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Component::StaticMesh>(_moto, "./resources/meshes/moto.glb");
	_scene.AddScript<RotateMoto>(_moto);
}