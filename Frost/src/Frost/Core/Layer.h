#pragma once

#include "Frost/Event/Event.h"

#include <string>

namespace Frost
{
	class Layer
	{
	public:
		Layer(const std::string& name) : _name(name) {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}
		virtual void OnLateUpdate(float deltaTime) {}

		const std::string& GetName() const { return _name; }

	private:
#ifdef FT_DEBUG
		std::string _name;
#endif
	};
}

