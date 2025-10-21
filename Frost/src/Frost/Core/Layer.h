#pragma once

#include "Frost/Event/Event.h"
#include "Frost/Core/Timer.h"

#include <string>

namespace Frost
{
	class Layer
	{
	public:
		using LayerName = std::string;
		using LayerPriority = uint32_t;

		Layer(const LayerName& name) : _name(name), _priority{ 0 } {}
		Layer(const LayerName& name, LayerPriority priority) : _name(name), _priority{ priority } {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}
		virtual void OnLateUpdate(float deltaTime) {}

		const LayerName& GetName() const { return _name; }

	private:
		LayerName _name;
		LayerPriority _priority;
	};
}

