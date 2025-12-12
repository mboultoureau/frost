#pragma once
#include "Frost/Scene/ECS/Component.h"
#include <memory>
#include <Frost/Asset/Texture.h>

namespace Frost::Component
{
    struct ScreenProjectedTexture : public Component
    {
        std::shared_ptr<Texture> texture = nullptr;
        bool enabled = true;
        bool hideGeometry = true;

        void SetRenderTarget(const std::shared_ptr<Texture>& texture) { this->texture = texture; }
    };
} // namespace Frost::Component