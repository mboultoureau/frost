#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Debugging/Assert.h"

namespace Frost
{
	class RendererAPI : NoCopy
	{
	public:
		static void SetRenderer(Renderer* renderer)
		{
			_renderer = renderer;
		}

		static void BeginFrame()
		{
			FT_ENGINE_ASSERT(_renderer != nullptr, "RendererAPI::BeginFrame called but no renderer is set!");
			_renderer->BeginFrame();
		}

		static void EndFrame()
		{
			FT_ENGINE_ASSERT(_renderer != nullptr, "RendererAPI::EndFrame called but no renderer is set!");
			_renderer->EndFrame();
		}

		static Renderer* GetRenderer()
		{
			FT_ENGINE_ASSERT(_renderer != nullptr, "RendererAPI::Get called but no renderer is set!");
			return _renderer;
		}

	private:
		static Renderer* _renderer;
	};
}