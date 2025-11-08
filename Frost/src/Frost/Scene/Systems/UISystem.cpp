#include "Frost/Scene/Systems/UISystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Input/Input.h"
#include <Frost/Scene/Components/UIButton.h>

namespace Frost
{
	UISystem::UISystem()
	{
	}

	void UISystem::Update(Frost::ECS& ecs, float deltaTime)
	{
		auto& buttons = ecs.GetIndexMap<UIButton>();

		POINT mouse;
		GetCursorPos(&mouse);
		ScreenToClient(Application::Get().GetWindow()->GetHWND(), &mouse);

		for (auto& b : buttons)
		{
			auto button = ecs.GetComponent<UIButton>(b);

			if (!button->enabled) continue;

			if (PtInRect(&(button->buttonHitbox), mouse) && button->onClick)
			{
				if (Input::GetMouse().IsButtonPressed(Mouse::MouseBoutton::Left))
				{
					button->texture = button->pressedTexture;
					button->onClick();
				}

				else
				{
					button->texture = button->hoverTexture;
				}
			}

			else
			{
				button->texture = button->idleTexture;
			}
		}
	}
}