#include "Frost/Scene/Systems/UISystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Input/Input.h"
#include <Frost/Scene/Components/UIButton.h>

namespace Frost
{
    UISystem::UISystem() {}

    void UISystem::Update(Scene& scene, float deltaTime)
    {
        auto buttonView = scene.ViewActive<Component::UIButton>();

        for (auto entity : buttonView)
        {
            auto [button] = buttonView.get(entity);

            if (Input::GetMouse().IsCursorInViewport(button.buttonHitbox) && button.onClick)
            {
                if (Input::GetMouse().IsButtonPressed(Mouse::MouseBoutton::Left))
                {
                    button.texture = button.pressedTexture;
                    button.onClick();
                }
                else
                {
                    button.texture = button.hoverTexture;
                }
            }
            else
            {
                button.texture = button.idleTexture;
            }
        }
    }

    void UISystem::LateUpdate(Scene& scene, float deltaTime)
    {
        auto buttonView = scene.ViewActive<Component::UIButton>();
        auto hudImageView = scene.ViewActive<Component::HUDImage>();

        _pipeline.BeginFrame();

        for (auto entity : hudImageView)
        {
            auto [image] = hudImageView.get(entity);
            _pipeline.Submit(image);
        }

        for (auto entity : buttonView)
        {
            auto [button] = buttonView.get(entity);
            _pipeline.Submit(button);
        }

        _pipeline.EndFrame();
    }
} // namespace Frost
