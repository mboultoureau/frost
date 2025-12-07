#pragma once

#include "Frost/Renderer/Pipeline.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Scene/Components/UIElement.h"

#include <vector>
#include <memory>

namespace Frost
{
    class CommandList;
    class Buffer;
    class Shader;
    class InputLayout;
    class Sampler;
    class Texture;

    class HUDTextRenderingPipeline : public Pipeline
    {
    public:
        HUDTextRenderingPipeline();
        virtual ~HUDTextRenderingPipeline();

        void Initialize();
        void Shutdown();

        void BeginFrame();
        void Submit(const Component::UIElement& element, const Component::UIText& text);
        void EndFrame();

        struct alignas(16) HUDShaderParameters
        {
            float viewport[4];
            float color[4];
        };

        struct HUD_Vertex
        {
            float x, y, z;
            float u, v;
        };

        struct DrawCall
        {
            Texture* texture;
            Material::FilterMode filter;
            HUDShaderParameters params;
            uint32_t vertexOffset;
            uint32_t vertexCount;
        };

    private:
        void SetFilter(Material::FilterMode filterMode);
        void _RegenerateMesh(const Component::UIElement& element, const Component::UIText& text);
        std::unique_ptr<CommandList> _commandList;

        std::shared_ptr<Shader> _vertexShader;
        std::shared_ptr<Shader> _pixelShader;
        std::unique_ptr<InputLayout> _inputLayout;

        std::shared_ptr<Buffer> _vertexBuffer;
        std::shared_ptr<Buffer> _constantBuffer;

        std::vector<HUD_Vertex> _accumulatedVertices;
        std::vector<DrawCall> _drawCalls;

        uint32_t _indexCount = 0;

        std::unique_ptr<Sampler> _samplerPoint;
        std::unique_ptr<Sampler> _samplerLinear;
        std::unique_ptr<Sampler> _samplerAnisotropic;

        bool _enabled = false;
    };
} // namespace Frost