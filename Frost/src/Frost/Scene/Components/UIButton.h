#pragma once

#include "Frost/Scene/Components/HUD_Image.h"
#include <Frost/Renderer/Viewport.h>
#include <functional>

namespace Frost
{
	struct UIButton : public HUD_Image
	{
		Texture* idleTexture = nullptr;
		Texture* hoverTexture = nullptr;
		Texture* pressedTexture = nullptr;

		std::string hoverTextureFilepath;
		std::string pressedTextureFilepath;

		RECT buttonHitbox;

		std::function<void()> onClick;

		UIButton(Viewport viewport, std::string pathMainTexture, std::string pathHoverTexture, std::string pathPressedTexture, std::function<void()> f) :
			HUD_Image(viewport, pathMainTexture, Material::FilterMode::POINT),
			hoverTextureFilepath(pathHoverTexture),
			pressedTextureFilepath(pathPressedTexture),
			onClick(f)
		{
			idleTexture = texture;
			hoverTexture = TextureLibrary::Get().GetTexture(pathHoverTexture, TextureType::HUD).get();
			pressedTexture = TextureLibrary::Get().GetTexture(pathPressedTexture, TextureType::HUD).get();
		}
	};
}