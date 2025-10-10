#pragma once

#include "Frost/Core/Layer.h"

#include <vector>
#include <memory>

namespace Frost
{
	class LayerStack
	{
	public:
		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void Clear();

	private:
		std::vector<Layer*> _layers;

		friend class Application;
	};
}
