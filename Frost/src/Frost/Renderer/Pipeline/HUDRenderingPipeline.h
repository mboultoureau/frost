#pragma once

#include "Frost/Renderer/Pipeline.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Scene/Components/UIElement.h"

namespace Frost
{
    class HUDRenderingPipeline : public Pipeline
    {
    public:
        HUDRenderingPipeline();
        virtual ~HUDRenderingPipeline();

        void Initialize();
        void Shutdown();

        void BeginFrame();
        void Submit(const Component::UIElement& element, const Component::UIImage& image);
        void EndFrame();

    private:
        struct alignas(16) HUDShaderParameters
        {
            float viewport[4];
            float color[4];
            float rotation;
            float padding[3];
        };

        void SetFilter(Material::FilterMode filterMode);

        std::unique_ptr<CommandList> _commandList;

        std::shared_ptr<Shader> _vertexShader;
        std::shared_ptr<Shader> _pixelShader;
        std::unique_ptr<InputLayout> _inputLayout;

        std::shared_ptr<Buffer> _vertexBuffer;
        std::shared_ptr<Buffer> _indexBuffer;
        std::shared_ptr<Buffer> _constantBuffer;
        uint32_t _indexCount = 0;

        std::unique_ptr<Sampler> _samplerPoint;
        std::unique_ptr<Sampler> _samplerLinear;
        std::unique_ptr<Sampler> _samplerAnisotropic;

        bool _enabled = false;
    };
} // namespace Frost