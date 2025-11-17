#include "LayerStack.h"

namespace Frost
{
	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find_if(_layers.begin(), _layers.end(),
			[layer](const std::unique_ptr<Layer>& l) { return l.get() == layer;
		});

		if (it != _layers.end())
		{
			(*it)->OnDetach();
			_layers.erase(it);
		}
	}

	void LayerStack::Clear()
	{
		for (const auto& layer : _layers)
		{
			layer->OnDetach();
		}
		_layers.clear();
	}
}

