#pragma once

#include "Frost/Scene/Scene.h"

namespace Frost
{
	class System
	{
	public:
		virtual ~System() = default;

		virtual void OnAttach(Scene& scene) {};
		virtual void OnDetach(Scene& scene) {};
		virtual void Update(Scene& scene, float deltaTime) {};
		virtual void LateUpdate(Scene& scene, float deltaTime) {};
		virtual void FixedUpdate(Scene& scene, float deltaTime) {};
		virtual void PreFixedUpdate(Scene& scene, float deltaTime) {};
	};
}
