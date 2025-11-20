#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/Pipeline.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/WorldTransform.h"

#include <memory>

namespace Frost
{
    class CommandList;
    class Texture;
    class Shader;
    class InputLayout;
    class Sampler;
    class Buffer;
    class Model;
    class Camera;
    class Light;
    struct WorldTransform;

	class DeferredRenderingPipeline : public Pipeline
    {
    public:
        DeferredRenderingPipeline();
        ~DeferredRenderingPipeline();

        void Initialize() override;
        void Shutdown() override;

        void BeginFrame(const Component::Camera& camera, const Math::Matrix4x4& viewMatrix, const Math::Matrix4x4& projectionMatrix);
        void SubmitModel(const Model& model, const Math::Matrix4x4& worldMatrix);
        void EndFrame(const Component::Camera& camera, const Component::WorldTransform& cameraTransform, const std::vector<std::pair<Component::Light, Component::WorldTransform>>& lights);

        Texture* GetDepthStencilTexture() const { return _depthStencilTexture.get(); }
        CommandList* GetCommandList() const { return _commandList.get(); }
        Texture* GetFinalLitTexture() const { return _finalLitTexture.get(); }

    private:
        void _CreateGBufferTextures(uint32_t width, uint32_t height);

        // G-Buffer Textures
        std::unique_ptr<Texture> _albedoTexture;
        std::unique_ptr<Texture> _normalTexture;
        std::unique_ptr<Texture> _worldPositionTexture;
        std::unique_ptr<Texture> _materialTexture;
        std::unique_ptr<Texture> _depthStencilTexture;
        std::unique_ptr<Texture> _finalLitTexture;

        // G-Buffer Pass Resources
        std::unique_ptr<Shader> _gBufferVertexShader;
        std::unique_ptr<Shader> _gBufferPixelShader;
        std::unique_ptr<InputLayout> _gBufferInputLayout;
        std::unique_ptr<Sampler> _materialSampler;

        // Lighting Pass Resources
        std::unique_ptr<Shader> _lightingVertexShader;
        std::unique_ptr<Shader> _lightingPixelShader;
        std::unique_ptr<InputLayout> _lightingInputLayout;
        std::unique_ptr<Sampler> _gBufferSampler;

        // Constant Buffers
        std::unique_ptr<Buffer> _vsPerFrameConstants;
        std::unique_ptr<Buffer> _vsPerObjectConstants;
        std::unique_ptr<Buffer> _lightConstantsBuffer;
		std::unique_ptr<Buffer> _psMaterialConstants;

        // Default Textures (used when material textures are missing)
        std::unique_ptr<Texture> _defaultAlbedoTexture;
        std::unique_ptr<Texture> _defaultNormalTexture;
        std::unique_ptr<Texture> _defaultMetallicTexture;
        std::unique_ptr<Texture> _defaultRoughnessTexture;
        std::unique_ptr<Texture> _defaultAOTexture;

        bool _enabled = true;

        void CreateDefaultTextures();

        std::unique_ptr<CommandList> _commandList;
    };
}