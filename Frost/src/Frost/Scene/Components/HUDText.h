#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Asset/Font.h"
#include "Frost/Utils/Math/Vector.h"

#include <string>
#include <memory>

namespace Frost::Component
{

    class HUDText : public Component
    {
    public:
        HUDText(Viewport viewport,
                std::shared_ptr<Font> font,
                std::string& text,
                Math::Vector4 color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f)) :
            viewport{ viewport }, text{ text }, font{ std::move(font) }, color{ color }
        {
        }
        float fontSize = 1.0f;
        std::shared_ptr<Font> font;
        std::string text;
        Viewport viewport;
        Math::Vector4 color;

        mutable bool isDirty = true;
    };
} // namespace Frost::Component