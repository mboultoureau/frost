#pragma once

#include "Frost/Renderer/Pipeline.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/WorldTransform.h"

namespace Frost
{
    class CommandList;
    class Texture;
    class Shader;
    class InputLayout;
    class Sampler;
    class Buffer;

    class SkyboxPipeline : public Pipeline
    {
    public:
        SkyboxPipeline();
        ~SkyboxPipeline();

        void Initialize() override;
        void Shutdown() override;

        void Render(CommandList* commandList, Texture* renderTarget, Texture* gbufferDepth, Texture* skyboxTexture, const Component::Camera& camera, const Component::WorldTransform& cameraTransform);

    private:
        void CreateCubeMesh();

        std::shared_ptr<Shader> _skyboxVertexShader;
        std::shared_ptr<Shader> _skyboxPixelShader;
        std::unique_ptr<InputLayout> _skyboxInputLayout;
        std::unique_ptr<Sampler> _skyboxSampler;

        std::shared_ptr<Buffer> _cubeVertexBuffer;
        std::shared_ptr<Buffer> _cubeIndexBuffer;
        uint32_t _cubeIndexCount;

        std::shared_ptr<Buffer> _vsSkyboxConstants;
    };
}