#pragma once

#include "Frost/Renderer/Renderer.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"

namespace Frost
{
	class RendererAPI
	{
	public:
		static void ClearColor(float r, float g, float b, float a = 1.0f);
		static void DrawFrame() { Get().DrawFrame(); }

		static Renderer& Get();
	};
}