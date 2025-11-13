#include "HierarchyTest.h"
#include "../Game.h"


using namespace Frost;

class PivotScript : public Frost::Script {
public:
	void OnFixedUpdate(float deltaTime) override
	{
		using namespace DirectX;
		auto scene = GetECS();
		auto transform = scene->GetComponent<Transform>(GetGameObject());;
		XMFLOAT4 currentRot = transform->rotation;
		XMVECTOR deltaRot = XMQuaternionRotationRollPitchYaw(0, deltaTime, 0);
		XMVECTOR qCurrent = XMLoadFloat4(&currentRot);
		XMVECTOR qResult = XMQuaternionMultiply(qCurrent, deltaRot);
		XMVECTOR qResultGlobal = XMQuaternionMultiply(deltaRot, qCurrent);
		qResult = XMQuaternionNormalize(qResult);
		XMStoreFloat4(&(transform->rotation), qResult);
	}
};


HierarchyTest::HierarchyTest()
{
	Scene& _scene = Game::GetScene();

	_parent = _scene.CreateGameObject("Parent");
	_scene.AddComponent<Frost::Transform>(_parent,
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_parent);
	_scene.AddComponent<ModelRenderer>(_parent, "./resources/meshes/sphere.fbx"); 
	_scene.AddScript<PivotScript>(_parent);

	_child = _scene.CreateGameObject("Child", _parent);
	_scene.AddComponent<Frost::Transform>(_child,
		Frost::Transform::Vector3{ 0.0f, 0.0f, 5.0f },
		Frost::Transform::Vector3{ -1.6f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_child);
	_scene.AddComponent<ModelRenderer>(_child, "./resources/meshes/cube.fbx");
}
