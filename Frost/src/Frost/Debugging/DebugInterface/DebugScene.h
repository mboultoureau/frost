#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"
#include "Frost/Scene/Scene.h"

#include <vector>

namespace Frost
{
	class DebugScene : public DebugPanel
	{
	public:
		DebugScene() = default;
		virtual ~DebugScene() override = default;
		virtual void OnImGuiRender(float deltaTime) override;
		virtual const char* GetName() const override { return "Scene"; }

		void AddScene(Scene* scene);
		void RemoveScene(Scene* scene);

	private:
		std::vector<Scene*> _scenes;
		float _deltaTime = 0.0f;

	private:
		void _DrawMetaComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawTransformComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawWorldTransformComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawStaticMeshComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawCameraComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawScriptableComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawLightComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawRigidBodyComponent(Scene* scene, GameObject::Id gameObjectId);

		void _DrawHUDImageComponent(Scene* scene, GameObject::Id gameObjectId);
		void _DrawUIButtonComponent(Scene* scene, GameObject::Id gameObjectId);
	};
}
