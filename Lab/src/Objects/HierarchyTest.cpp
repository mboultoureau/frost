#include "HierarchyTest.h"
#include "../Game.h"


using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class PivotScript : public Frost::Script {
public:
	void OnFixedUpdate(float deltaTime) override
	{
		using namespace DirectX;
		auto& transform = GetGameObject().GetComponent<Transform>();
		XMFLOAT4 currentRot = vector_cast<XMFLOAT4>(transform.rotation);
		XMVECTOR deltaRot = XMQuaternionRotationRollPitchYaw(0, deltaTime, 0);
		XMVECTOR qCurrent = XMLoadFloat4(&currentRot);
		XMVECTOR qResult = XMQuaternionMultiply(qCurrent, deltaRot);
		XMVECTOR qResultGlobal = XMQuaternionMultiply(deltaRot, qCurrent);
		qResult = XMQuaternionNormalize(qResult);
		transform.rotation = vector_cast<Vector4>(qResult);
	}
};


HierarchyTest::HierarchyTest()
{
	Scene& _scene = Game::GetScene();

	_parent = _scene.CreateGameObject("Parent");
	_scene.AddComponent<Transform>(_parent,
		Vector3{ 0.0f, 0.0f, 0.0f }
	);
	_scene.AddComponent<WorldTransform>(_parent);
	_scene.AddComponent<StaticMesh>(_parent, "./resources/meshes/sphere.fbx"); 
	_scene.AddScript<PivotScript>(_parent);

	_child = _scene.CreateGameObject("Child", _parent);
	_scene.AddComponent<Transform>(_child,
		Vector3{ 0.0f, 0.0f, 5.0f },
		EulerAngles{ -90.0_deg, 0.0f, 0.0f },
		Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<WorldTransform>(_child);
	_scene.AddComponent<StaticMesh>(_child, "./resources/meshes/cube.fbx");
}
