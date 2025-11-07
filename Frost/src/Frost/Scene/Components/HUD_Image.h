#pragma once

#include "Frost/Scene/ECS/Component.h"
#include <DirectXMath.h> 
#include "Frost/Renderer/TextureLibrary.h" 

#include <string>
#include <memory> 

namespace Frost
{
    using Vector2 = DirectX::XMFLOAT2;

    struct HUD_Image : public Component
    {
        Texture* texture = nullptr;

        std::string textureFilepath;
       
        Viewport viewport;

        HUD_Image() = default;

        HUD_Image(Viewport viewport,std::string pathTexture)
            : viewport{viewport}, textureFilepath{ pathTexture }
        {
            texture = TextureLibrary::Get().GetTexture(pathTexture, TextureType::HUD).get();
        }
    };
}