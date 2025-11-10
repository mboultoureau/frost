#include "PauseMenu.h"
#include "Frost/Core/Application.h"
#include <Frost/Input/Devices/Keyboard.h>
#include <Frost/Input/Input.h>
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include <Frost/Debugging/Logger.h>

namespace Frost
{
	PauseMenu::PauseMenu() : Layer(GetStaticName()),
		_resetButtonReleased(false),
		_pauseButtonReleased(true),
		_gamePaused(false)
	{
	}

	void PauseMenu::OnAttach()
	{
		if (_scenes.size() == 0) return;

		pauseTextId = _scenes.at(0)->CreateGameObject("pause text");
		resumeButtonId = _scenes.at(0)->CreateGameObject("resume button");
		resetButtonId = _scenes.at(0)->CreateGameObject("reset button");

		pauseNRect = { 0.5f, -100, 200, 50 };
		resumeNRect = { 0.5f, -50, 200, 50 };
		resetNRect = { 0.5f, 0, 200, 50 };

		AddMenuComponents();

		HideMenu();
	}

	void PauseMenu::AddMenuComponents()
	{
		Viewport textViewport;
		Viewport resumeViewport;
		Viewport resetViewport;

		_scenes.at(0)->AddComponent<Frost::HUD_Image>(
			pauseTextId,
			textViewport,
			pausePath,
			Material::FilterMode::POINT
		);

		_scenes.at(0)->AddComponent<Frost::UIButton>(
			resumeButtonId,
			resumeViewport,
			idleResumePath,
			hoverResumePath,
			hoverResumePath,
			[this]() {OnUnpauseButtonPress(); }
		);

		_scenes.at(0)->AddComponent<Frost::UIButton>(
			resetButtonId,
			resetViewport,
			idleResetPath,
			hoverResetPath,
			hoverResetPath,
			[this]() {OnResetButtonPress(); }
		);
	}

	void PauseMenu::OnDetach()
	{
	}

	void PauseMenu::OnUpdate(float deltaTime)
	{
	}

	void PauseMenu::OnLateUpdate(float deltaTime)
	{
	}

	void PauseMenu::OnFixedUpdate(float fixedDeltaTime)
	{
		RECT client;
		GetClientRect(Application::Get().GetWindow()->GetHWND(), &client);
		int w = client.right - client.left;
		int h = client.bottom - client.top;

		auto resumeButton = _scenes.at(0)->GetComponent<Frost::UIButton>(resumeButtonId);
		auto resetButton = _scenes.at(0)->GetComponent<Frost::UIButton>(resetButtonId);
		auto pauseText = _scenes.at(0)->GetComponent<Frost::HUD_Image>(pauseTextId);

		RECT resumeRect = SetImgDimensionsFromWindowSize(resumeButton, resumeNRect, w, h);
		resumeButton->buttonHitbox = resumeRect;

		RECT resetRect = SetImgDimensionsFromWindowSize(resetButton, resetNRect, w, h);
		resetButton->buttonHitbox = resetRect;

		SetImgDimensionsFromWindowSize(pauseText, pauseNRect, w, h);

		ProcessInput();
	}

	RECT PauseMenu::SetImgDimensionsFromWindowSize(HUD_Image* img, NormalizedRect imgRect, int w, int h)
	{
		int pxWidth = static_cast<int>(imgRect.width);
		int pxHeight = static_cast<int>(imgRect.height);

		int centerX = static_cast<int>(imgRect.x * w);
		int centerY = static_cast<int>(imgRect.y + h / 2);

		RECT rect;
		rect.left = centerX - pxWidth / 2;
		rect.top = centerY - pxHeight / 2;
		rect.right = centerX + pxWidth / 2;
		rect.bottom = centerY + pxHeight / 2;

		img->viewport.width = imgRect.width / w;
		img->viewport.height = imgRect.height / h;
		img->viewport.y = rect.top / static_cast<float>(h);
		img->viewport.x = rect.left / static_cast<float>(w);

		return rect;
	}

	void PauseMenu::ProcessInput()
	{
		if (Input::GetKeyboard().IsKeyDown(K_ESCAPE) && _pauseButtonReleased)
		{
			//FT_ENGINE_INFO("Pause Toggled");
			_pauseButtonReleased = false;
			_gamePaused = !_gamePaused;

			if (_gamePaused)
			{
				Application::Get().GetEventManager().Emit<Frost::PauseEvent>();
				ShowMenu();
			}
			else
			{
				Application::Get().GetEventManager().Emit<Frost::UnPauseEvent>();
				HideMenu();
			}
		}
		else if(!Input::GetKeyboard().IsKeyDown(K_ESCAPE))
		{
			//FT_ENGINE_INFO("Pause Released");
			_pauseButtonReleased = true;
		}

		if (Input::GetKeyboard().IsKeyDown(K_R) && _resetButtonReleased)
		{
			Application::Get().GetEventManager().Emit<Frost::ResetEvent>();
			_resetButtonReleased = false;
		}
		else if (!Input::GetKeyboard().IsKeyDown(K_R))
		{
			_resetButtonReleased = true;
		}
	}

	void PauseMenu::ShowMenu()
	{
		_scenes.at(0)->GetComponent<Frost::HUD_Image>(pauseTextId)->enabled = true;
		_scenes.at(0)->GetComponent<Frost::UIButton>(resumeButtonId)->enabled = true;
		_scenes.at(0)->GetComponent<Frost::UIButton>(resetButtonId)->enabled = true;
	}

	void PauseMenu::HideMenu()
	{
		_scenes.at(0)->GetComponent<Frost::HUD_Image>(pauseTextId)->enabled = false;
		_scenes.at(0)->GetComponent<Frost::UIButton>(resumeButtonId)->enabled = false;
		_scenes.at(0)->GetComponent<Frost::UIButton>(resetButtonId)->enabled = false;
	}

	void PauseMenu::OnUnpauseButtonPress()
	{
		Application::Get().GetEventManager().Emit<Frost::UnPauseEvent>();
		HideMenu();
		_gamePaused = !_gamePaused;
	}

	void PauseMenu::OnResetButtonPress()
	{
		Application::Get().GetEventManager().Emit<Frost::ResetEvent>();
		_resetButtonReleased = false;
	}
}