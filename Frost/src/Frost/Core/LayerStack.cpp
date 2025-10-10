#include "LayerStack.h"

namespace Frost
{
	void LayerStack::PushLayer(Layer* layer)
	{
		_layers.emplace_back(layer);
		layer->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(_layers.begin(), _layers.end(), layer);
		if (it != _layers.end())
		{
			layer->OnDetach();
			_layers.erase(it);
		}
	}

	void LayerStack::Clear()
	{
		for (Layer* layer : _layers)
		{
			layer->OnDetach();
		}
		_layers.clear();
	}
}

