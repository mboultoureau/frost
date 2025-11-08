#include "PauseMenu.h"
#include "Frost/Core/Application.h"
#include <Frost/Input/Devices/Keyboard.h>
#include <Frost/Input/Input.h>
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include <Frost/Debugging/Logger.h>
#include <Frost/Scene/Components/HUD_Image.h>

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

		menuId = _scenes.at(0)->CreateGameObject("pause hud");

		const std::string LOGO_PATH = "resources/meshes/pause.png";

		Viewport viewport;
		viewport.height = 0.0f;
		viewport.width = 0.2f;
		viewport.x = 0.4f;
		viewport.y = 0.4f;

		_scenes.at(0)->AddComponent<Frost::HUD_Image>(
			menuId,
			viewport,
			LOGO_PATH,
			Material::FilterMode::POINT
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
		ProcessInput();
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
		auto pauseHud = _scenes.at(0)->GetComponent<Frost::HUD_Image>(menuId);
		pauseHud->viewport.height = 0.2;
	}

	void PauseMenu::HideMenu()
	{
		auto pauseHud = _scenes.at(0)->GetComponent<Frost::HUD_Image>(menuId);
		pauseHud->viewport.height = 0;
	}
}