#pragma once

#include "Frost/Utils/Math/NormalizedRect.h"
#include "Frost/Core/Layer.h"
#include <Frost/Scene/Scene.h>
#include <Frost/Event/Events/Input/KeyPressedEvent.h>
#include <Frost/Scene/Components/UIButton.h>

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
		const std::string pausePath = "resources/textures/pause-text.png";
		GameObject::Id pauseTextId;
		NormalizedRect pauseNRect;

		const std::string idleResumePath = "resources/textures/unpause-idle.png";
		const std::string hoverResumePath = "resources/textures/unpause-hover.png";
		const std::string pressedResumePath = "resources/textures/unpause-press.png";
		GameObject::Id resumeButtonId;
		NormalizedRect resumeNRect;

		const std::string idleResetPath = "resources/textures/reset-idle.png";
		const std::string hoverResetPath = "resources/textures/reset-hover.png";
		const std::string pressedResetPath = "resources/textures/reset-press.png";
		GameObject::Id resetButtonId;
		NormalizedRect resetNRect;

		void AddMenuComponents();
		RECT SetImgDimensionsFromWindowSize(HUD_Image* img, NormalizedRect imgRect, int w, int h);
		void ProcessInput();
		void ShowMenu();
		void HideMenu();
		void OnUnpauseButtonPress();
		void OnResetButtonPress();
		bool _resetButtonReleased;
		bool _pauseButtonReleased;
		bool _gamePaused;
		std::vector<Scene*> _scenes;
	};
}