#pragma once

#include "Frost/Renderer/GPUResource.h"

namespace Frost
{
	class RenderPass : public GPUResource
	{
	public:
		RenderPass() = default;
		virtual ~RenderPass() = default;
	};
}