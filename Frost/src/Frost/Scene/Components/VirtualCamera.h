#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Scene/Components/Camera.h"

namespace Frost::Component
{
	class VirtualCamera : public Camera
	{
	public:
		const std::shared_ptr<Texture>& GetRenderTarget() const { return renderTarget; }
		void SetRenderTarget(const std::shared_ptr<Texture>& texture) { renderTarget = texture; }

		enum class RespectRatio
		{
			Width,
			Height
		};

		float aspectRatio = 1.0f;
		int width = 100;
		int height = 100;

		RespectRatio respectRatio = RespectRatio::Width;
		std::shared_ptr<Texture> renderTarget;

		bool useScreenSpaceAspectRatio = false;
	};
}