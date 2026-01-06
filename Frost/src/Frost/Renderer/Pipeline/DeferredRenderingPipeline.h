#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/Pipeline.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Renderer/Frustum.h"

#include <memory>
#include <unordered_map>

namespace Frost
{
    class CommandList;
    class Texture;
    class Shader;
    class InputLayout;
    class Sampler;
    class Buffer;
    class Model;
    class Component::Camera;
    class Component::Light;
    struct Component::WorldTransform;

    class DeferredRenderingPipeline : public Pipeline
    {
    public:
        DeferredRenderingPipeline();
        ~DeferredRenderingPipeline();

        void Initialize() override;
        void Shutdown() override;
        void OnWindowResize(WindowResizeEvent& e) override;

        void OnResize(uint32_t width, uint32_t height);

        void BeginFrame(const Component::Camera& camera,
                        const Component::WorldTransform& cameraTransform,
                        const Math::Matrix4x4& viewMatrix,
                        const Math::Matrix4x4& projectionMatrix,
                        const Viewport& viewport);
        void SubmitModel(const Model& model, const Math::Matrix4x4& worldMatrix);
        /* void EndFrame(const Component::Camera& camera,
                      const Component::WorldTransform& cameraTransform,
                      const std::vector<std::pair<Component::Light, Component::WorldTransform>>& lights,
                      const Viewport& viewport);
        */

        std::shared_ptr<Texture> GetAlbedoTexture() const { return _albedoTexture; }
        std::shared_ptr<Texture> GetNormalTexture() const { return _normalTexture; }
        std::shared_ptr<Texture> GetWorldPositionTexture() const { return _worldPositionTexture; }
        std::shared_ptr<Texture> GetMaterialTexture() const { return _materialTexture; }
        std::shared_ptr<Texture> GetDepthStencilTexture() const { return _depthStencilTexture; }
        std::shared_ptr<Texture> GetEmissionTexture() const { return _emissionTexture; }

        CommandList* GetCommandList() const { return _commandList.get(); }
        std::shared_ptr<CommandList> GetSharedCommandList() const { return _commandList; }

    private:
        void _CreateGBufferTextures(uint32_t width, uint32_t height);

        // G-Buffer Textures
        std::shared_ptr<Texture> _albedoTexture;
        std::shared_ptr<Texture> _normalTexture;
        std::shared_ptr<Texture> _worldPositionTexture;
        std::shared_ptr<Texture> _materialTexture;
        std::shared_ptr<Texture> _emissionTexture;
        std::shared_ptr<Texture> _depthStencilTexture;

        // G-Buffer Pass Resources
        std::shared_ptr<Shader> _gBufferVertexShader;
        std::shared_ptr<Shader> _gBufferPixelShader;
        std::unique_ptr<InputLayout> _gBufferInputLayout;
        std::unique_ptr<Sampler> _materialSampler;

        // Lighting Pass Resources
        /* std::shared_ptr<Shader> _lightingVertexShader;
        std::shared_ptr<Shader> _lightingPixelShader;
        std::unique_ptr<InputLayout> _lightingInputLayout;*/
        std::unique_ptr<Sampler> _gBufferSampler;

        // Constant Buffers
        std::shared_ptr<Buffer> _vsPerFrameConstants;
        std::shared_ptr<Buffer> _vsPerObjectConstants;
        // std::shared_ptr<Buffer> _lightConstantsBuffer;
        std::shared_ptr<Buffer> _psMaterialConstants;

        // Default Textures (used when material textures are missing)
        std::unique_ptr<Texture> _defaultAlbedoTexture;
        std::unique_ptr<Texture> _defaultNormalTexture;
        std::unique_ptr<Texture> _defaultMetallicTexture;
        std::unique_ptr<Texture> _defaultRoughnessTexture;
        std::unique_ptr<Texture> _defaultAOTexture;
        std::unique_ptr<Texture> _defaultEmissionTexture;

        // Materials buffers
        std::shared_ptr<Buffer> _customMaterialConstantBuffer;
        std::unordered_map<Shader*, std::unique_ptr<InputLayout>> _inputLayoutCache;

        std::shared_ptr<CommandList> _commandList;

        bool _enabled = true;
        uint32_t _currentWidth = 0;
        uint32_t _currentHeight = 0;

    private:
        void _CreateDefaultTextures();
        InputLayout* _GetOrCreateInputLayout(Shader* vertexShader);
    };
} // namespace Frost