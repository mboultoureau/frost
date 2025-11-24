#pragma once

#include "Frost/Renderer/Pipeline.h"
#include "Frost/Scene/Components/HUDImage.h"
#include "Frost/Scene/Components/UIButton.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Vector.h"

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
        void Submit(const Component::HUDImage& image);
        void Submit(const Component::UIButton& button);
        void EndFrame();

    private:
        struct alignas(16) HUDShaderParameters
        {
            float viewport[4];
        };

        void SetFilter(Material::FilterMode filterMode);

        std::unique_ptr<CommandList> _commandList;

        std::unique_ptr<Shader> _vertexShader;
        std::unique_ptr<Shader> _pixelShader;
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
}