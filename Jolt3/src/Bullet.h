#pragma once
#include <DirectXMath.h>
#include "Frost/Scene/Components/Script.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/ECS/GameObject.h"
#include <Frost/Scene/Components/GameObjectInfo.h>
#include <Frost/Scene/Scene.h>

struct BulletFactory {
	using Vector3 = DirectX::XMFLOAT3;
	Frost::GameObject::Id operator()(Frost::Scene& scene, Vector3& dir, float speed, Frost::GameObjectInfo& parent);
	Frost::GameObject::Id operator()(Frost::Scene& scene, Vector3& dir, float speed, Frost::GameObject::Id& parentId);
	Frost::GameObject::Id operator()(Frost::Scene& scene, Vector3& dir, float speed);
};

class BulletScript : Frost::Script {
	using Vector3 = DirectX::XMFLOAT3;
	float _timer;
	float _lifeDuration; //todo : initialize
	Vector3 _dir;

	//Components :
	//Frost::Body* _body;
	Frost::GameObjectInfo& _goInfo;

public:
	BulletScript(const float& lifeDuration, const float& speed, const Vector3& dir) noexcept
		: _dir(speed* dir.x, speed* dir.y, speed* dir.z), _timer{ 0.f }, _lifeDuration{ lifeDuration }, _goInfo{ *GetECS()->GetComponent<Frost::GameObjectInfo>(GetGameObject()) }
	{
	}

	void OnAwake() override;
	void OnUpdate(float deltaTime) override;

};
