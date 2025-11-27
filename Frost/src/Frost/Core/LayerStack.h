#pragma once

#include "Frost/Core/Layer.h"

#include <memory>
#include <vector>

namespace Frost
{
    class LayerStack
    {
    public:
        using LayerContainer = std::vector<std::unique_ptr<Layer>>;

        template<typename T, typename... Args>
        T* PushLayer(Args&&... args)
        {
            static_assert(std::is_base_of<Layer, T>::value, "T must be derived from Frost::Layer");

            auto layer = std::make_unique<T>(std::forward<Args>(args)...);
            T* layerPtr = layer.get();

            layer->OnAttach();

            _layers.emplace_back(std::move(layer));
            return layerPtr;
        }

        void PopLayer(Layer* layer);
        void Clear();

        LayerContainer::iterator begin() { return _layers.begin(); }
        LayerContainer::iterator end() { return _layers.end(); }
        LayerContainer::const_iterator begin() const { return _layers.cbegin(); }
        LayerContainer::const_iterator end() const { return _layers.cend(); }

    private:
        LayerContainer _layers;
    };
} // namespace Frost
