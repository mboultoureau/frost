#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
	void RendererAPI::ClearColor(float r, float g, float b, float a)
	{
		Get().ClearColor(r, g, b, a);
	}

	void RendererAPI::DrawMesh(const MeshRenderer& mesh, const WorldTransform& transform)
	{

	}

	Renderer& RendererAPI::Get()
	{
		static Renderer _renderer;
		return _renderer;
	}
}