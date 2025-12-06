/*
#pragma once

#include "Frost/Renderer/Pipeline.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Renderer/Buffer.h"

#include <memory>

namespace Frost
{
    class CommandList;
    class Shader;
    class Buffer;
    class Sampler;

    struct alignas(16) VS_PerFrameConstants
    {
        Frost::Math::Matrix4x4 ViewMatrix;
        Frost::Math::Matrix4x4 ProjectionMatrix;
    };

    struct alignas(16) BillboardConstants
    {
        Frost::Math::Vector3 Position;
        float Width;

        float Height;
        float CenteredY;
        Frost::Math::Vector2 Padding;
    };

    class BillboardRenderingPipeline : public Pipeline
    {
    public:
        BillboardRenderingPipeline();
        virtual ~BillboardRenderingPipeline();

        void Initialize();
        void Shutdown();

        void BeginFrame(const Frost::Math::Matrix4x4& view,
                        const Frost::Math::Matrix4x4& projection,
                        Texture* renderTarget,
                        Texture* depthStencil);

        void Submit(const BillboardConstants& constants,
                    std::shared_ptr<Texture> texture,
                    Material::FilterMode textureFilter);

        void EndFrame();

    private:
        void _SetFilter(Material::FilterMode filterMode);

        std::unique_ptr<CommandList> _commandList;

        std::shared_ptr<Shader> _vertexShader;
        std::shared_ptr<Shader> _pixelShader;

        std::shared_ptr<Buffer> _perFrameConstantBuffer;
        std::shared_ptr<Buffer> _perDrawConstantBuffer;

        std::shared_ptr<Sampler> _samplerPoint;
        std::shared_ptr<Sampler> _samplerLinear;

        bool _enabled = false;
    };
} // namespace Frost*/