#include "Frost/Scene/Systems/UISystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Input/Input.h"
#include "Frost/Scene/Components/UIElement.h"

#include <algorithm>
#include <vector>

namespace Frost
{
    UISystem::UISystem() {}

    UISystem::~UISystem() {}

    void UISystem::Update(Scene& scene, float deltaTime)
    {
        auto uiView = scene.ViewActive<Component::UIElement>();
        const Mouse& mouse = Input::GetMouse();

        for (auto entity : uiView)
        {
            auto& element = uiView.get<Component::UIElement>(entity);
            if (!element.isEnabled)
                continue;

            if (auto* button = std::get_if<Component::UIButton>(&element.content))
            {
                if (mouse.IsCursorInViewport(element.viewport))
                {
                    if (mouse.IsButtonPressed(Mouse::MouseBoutton::Left))
                    {
                        button->state = Component::ButtonState::Pressed;
                        if (button->onClick)
                        {
                            button->onClick();
                        }
                    }
                    else
                    {
                        button->state = Component::ButtonState::Hover;
                    }
                }
                else
                {
                    button->state = Component::ButtonState::Idle;
                }
            }
        }
    }

    void UISystem::LateUpdate(Scene& scene, float deltaTime)
    {
        auto uiView = scene.ViewActive<Component::UIElement>();
        std::vector<Component::UIElement*> elementsToRender;

        for (auto entity : uiView)
        {
            auto& element = uiView.get<Component::UIElement>(entity);
            if (element.isEnabled)
            {
                elementsToRender.push_back(&element);
            }
        }

        std::sort(elementsToRender.begin(),
                  elementsToRender.end(),
                  [](const Component::UIElement* a, const Component::UIElement* b)
                  { return a->priority < b->priority; });

        _pipeline.BeginFrame();
        _pipelineText.BeginFrame();

        for (const auto* element : elementsToRender)
        {
            std::visit(
                [&](auto&& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, Component::UIImage>)
                    {
                        if (arg.texture)
                            _pipeline.Submit(*element, arg);
                    }
                    else if constexpr (std::is_same_v<T, Component::UIText>)
                    {
                        if (arg.font)
                            _pipelineText.Submit(*element, arg);
                    }
                    else if constexpr (std::is_same_v<T, Component::UIButton>)
                    {
                        std::shared_ptr<Texture> textureToDraw = arg.idleTexture;
                        if (arg.state == Component::ButtonState::Hover && arg.hoverTexture)
                        {
                            textureToDraw = arg.hoverTexture;
                        }
                        else if (arg.state == Component::ButtonState::Pressed && arg.pressedTexture)
                        {
                            textureToDraw = arg.pressedTexture;
                        }

                        if (textureToDraw)
                        {
                            Component::UIImage imageData{ textureToDraw };
                            _pipeline.Submit(*element, imageData);
                        }
                    }
                },
                element->content);
        }

        _pipeline.EndFrame();
        _pipelineText.EndFrame();
    }
} // namespace Frost