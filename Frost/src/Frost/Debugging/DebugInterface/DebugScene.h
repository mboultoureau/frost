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
		entt::entity _selectedEntity{ entt::null };

		bool _reparentingRequested = false;
		entt::entity _entityToReparent = entt::null;
		entt::entity _newParentOfEntity = entt::null;

	private:
		void _DrawHierarchyPanel();
		void _DrawInspectorPanel();

		void _DrawEntityNode(Scene* scene, entt::entity gameObjectId);
		void _ReparentEntity(Scene* scene, entt::entity entityId, entt::entity newParentId);

		void _DrawMetaComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawTransformComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawWorldTransformComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawStaticMeshComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawCameraComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawScriptableComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawLightComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawRigidBodyComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawHUDImageComponent(Scene* scene, entt::entity gameObjectId);
		void _DrawUIButtonComponent(Scene* scene, entt::entity gameObjectId);
	};
}
