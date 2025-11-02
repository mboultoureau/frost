#pragma once

#include "Frost/Renderer/Texture.h"

#include <string>
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace Frost
{
	struct Material
	{
		std::string name;
		DirectX::XMFLOAT3 diffuseColor;

		std::vector<std::shared_ptr<Texture>> diffuseTextures;
	};
}


