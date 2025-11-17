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
		auto& buttons = ecs.GetIndexMap<Component::UIButton>();

		for (auto& b : buttons)
		{
			auto button = ecs.GetComponent<Component::UIButton>(b);

			if (!button->isEnabled) continue;

			if (Input::GetMouse().IsCursorInViewport(button->buttonHitbox) && button->onClick)
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

	void UISystem::LateUpdate(ECS& ecs, float deltaTime)
	{
		const auto& hudImages = ecs.GetDataArray<Component::HUDImage>();
		const auto& uiButtons = ecs.GetDataArray<Component::UIButton>();

		if (hudImages.empty() && uiButtons.empty())
		{
			return;
		}

		_pipeline.BeginFrame();

		for (const auto& image : hudImages)
		{
			if (image.IsEnabled())
			{
				_pipeline.Submit(image);
			}
		}

		for (const auto& button : uiButtons)
		{
			if (button.IsEnabled())
			{
				_pipeline.Submit(button);
			}
		}

		_pipeline.EndFrame();
	}
}
