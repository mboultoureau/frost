#include "Frost/Scene/Systems/RendererSystem.h"

#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
	void RendererSystem::Update(Frost::ECS& ecs, float deltaTime)
	{
		Render(ecs);
	}

    void RendererSystem::Render(ECS& ecs)
    {
        const auto& renderers = ecs.GetDataArray<MeshRenderer>();
        const auto& rendererEntities = ecs.GetIndexMap<MeshRenderer>();

        for (size_t i = 0; i < renderers.size(); ++i)
        {
            const MeshRenderer& mesh = renderers[i];
            GameObject::Id id = rendererEntities[i];

            WorldTransform* transform = ecs.GetComponent<WorldTransform>(id);

            if (transform)
            {
				RendererAPI::DrawMesh(mesh, *transform);
            }
        }
    }
}

