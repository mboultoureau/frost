#pragma once

#include "Frost/Core/Layer.h"
#include <Frost/Scene/Scene.h>
#include <Frost/Event/Events/Input/KeyPressedEvent.h>

namespace Frost
{
	class PauseMenu : public Layer
	{
	public:
		PauseMenu();

		static Layer::LayerName GetStaticName() { return "PauseMenuLayer"; }

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnLateUpdate(float deltaTime) override;
		void OnFixedUpdate(float fixedDeltaTime) override;

		void AddScene(Scene* scene) { _scenes.push_back(scene); }

	private:
		void ProcessInput();
		void ShowMenu();
		void HideMenu();
		bool _resetButtonReleased;
		bool _pauseButtonReleased;
		bool _gamePaused;
		std::vector<Scene*> _scenes;
		Frost::GameObject::Id menuId;
	};
}