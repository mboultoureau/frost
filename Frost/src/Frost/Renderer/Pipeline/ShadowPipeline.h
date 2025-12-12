#pragma once

#include "Frost/Renderer/Pipeline.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/CommandList.h"
#include "Frost/Core/Core.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Renderer/Frustum.h"

#include <memory>
#include <unordered_map>
#include "DeferredRenderingPipeline.h"

namespace Frost
{
    class CommandList;
    class Texture;
    class Shader;
    class InputLayout;
    class Sampler;
    class Buffer;

    struct LightObject
    {
        LightObject(int id, Component::WorldTransform& wt, Component::Light& light) : id{ id }, wt{ wt }, light{ light }
        {
        }

        int id;
        Component::WorldTransform& wt;
        Component::Light& light;
    };

    struct ShadowData
    {
        std::unique_ptr<Texture> shadowTexture;
        Math::Matrix4x4 lightViewProj;
        Frustum lightFrustum; // Cache du frustum pour éviter de le recalculer
    };

    struct DirectionalShadowData : public ShadowData
    {
        float cameraNear;
        float cameraFar;
        float cameraFOV;
        const Component::WorldTransform& cameraTransform;
    };

    // Virtual
    struct LightData
    {
    };

    class ShadowPipeline : public Pipeline
    {
    public:
        ShadowPipeline();
        ~ShadowPipeline();

        void OnWindowResize(WindowResizeEvent& e) override;

        void InitLightShaders();

        void Initialize() override;
        void Shutdown() override;
        void OnResize(uint32_t width, uint32_t height);
        void SetGBufferData(DeferredRenderingPipeline* deferredPipeline, Scene* scene);

        Texture* GetFinalLitTexture() { return _finalLitTexture.get(); };

        void MakePointDirectionalLight(Math::EulerAngles rot,
                                       std::pair<Component::Light, Component::WorldTransform> lightPair);

        void MakeScaledDirectionalLight(std::pair<Component::Light, Component::WorldTransform> lightPair,
                                        float cameraNear,
                                        float cameraFar,
                                        float cameraFOV,
                                        const Component::WorldTransform& cameraTransform);

        void ShadowPass(std::vector<std::pair<Component::Light, Component::WorldTransform>> lightPairs,
                        const Component::Camera& camera,
                        const Component::WorldTransform& cameraTransform,
                        const Viewport& viewport);
        void LightPass(const Component::Camera& camera,
                       const Component::WorldTransform& cameraTransform,
                       const Viewport& viewport);

        void InitLightTexture(const Viewport& viewport);

    private:
        void CreateTextures(uint32_t width, uint32_t height);

        void ComputeShadowMap(const LightObject& lightObject,
                              const Component::Camera& camera,
                              const Component::WorldTransform& cameraTransform,
                              const Viewport& viewport);

        void ComputeDirectionalShadowMap(const LightObject& lightObj,
                                         const Component::WorldTransform& cameraTransform,
                                         const Viewport& viewport);

        struct DirectionalParams
        {
            Math::Vector3 sunPos;
            float orthoSize;
            float nearPlane;
            float farPlane;
        };
        DirectionalParams ComputeOrthoSize(float cameraNear,
                                           float cameraFar,
                                           float cameraFOV,
                                           const Component::WorldTransform& cameraTransform,
                                           const Component::WorldTransform& sunTransform);

        // Optimisation : Ajout du paramètre Frustum
        void DrawDepthOnly(CommandList* cmd,
                           const Component::StaticMesh& staticMesh,
                           const Math::Matrix4x4& worldMatrix,
                           const Math::Matrix4x4& _currentLightViewProj,
                           const Frustum& lightFrustum);

        void SubmitLight(const Component::Camera& camera,
                         const Component::WorldTransform& cameraTransform,
                         LightObject lightObj,
                         std::shared_ptr<Texture> destination,
                         std::shared_ptr<Texture> source,
                         const Viewport& viewport);

        void DrawLight(ShadowData& shadow,
                       std::shared_ptr<Shader> lightingVertexShader,
                       std::shared_ptr<Shader> lightingPixelShader,
                       std::shared_ptr<Texture> destination,
                       std::shared_ptr<Texture> source,
                       const Viewport& viewport);

        void DrawFinalLitTexture(std::shared_ptr<Texture> luminanceTexture, const Viewport& viewport);

        int _shadowResolution = 2048; // Peut être réduit à 1024 si besoin de perf
        float _orthoSize = 512;
        int _currentWidth = 0;
        int _currentHeight = 0;

        std::vector<std::pair<Component::Light, Component::WorldTransform>> _virtualLightPairs;
        std::unordered_map<int, ShadowData> _shadowMaps;

        Scene* _scene;

        // Shaders
        std::shared_ptr<Shader> _shadowVertexShader;
        std::shared_ptr<Shader> _shadowPixelShader;

        std::shared_ptr<Shader> _pointLightVertexShader;
        std::shared_ptr<Shader> _pointLightPixelShader;

        std::shared_ptr<Shader> _directionalLightVertexShader;
        std::shared_ptr<Shader> _directionalLightPixelShader;

        std::shared_ptr<Shader> _ambiantLightVertexShader;
        std::shared_ptr<Shader> _ambiantLightPixelShader;

        std::shared_ptr<Shader> _spotLightVertexShader;
        std::shared_ptr<Shader> _spotLightPixelShader;

        std::shared_ptr<Shader> _initLightPixelShader;
        std::shared_ptr<Shader> _finalLightVertexShader;
        std::shared_ptr<Shader> _finalLightPixelShader;

        // Shader ressources
        std::shared_ptr<Buffer> _vsShadowConstants;
        std::shared_ptr<Buffer> _lightPassBuffer;

        std::unique_ptr<Sampler> _shadowSampler;
        std::unique_ptr<Sampler> _gBufferSampler;
        std::unique_ptr<InputLayout> _shadowInputLayout;
        std::shared_ptr<CommandList> _commandList;

        // G-Buffer textures
        std::shared_ptr<Texture> _albedoTexture;
        std::shared_ptr<Texture> _normalTexture;
        std::shared_ptr<Texture> _worldPositionTexture;
        std::shared_ptr<Texture> _materialTexture;

        std::shared_ptr<Texture> _luminanceTexture1;
        std::shared_ptr<Texture> _luminanceTexture2;

        std::unique_ptr<Texture> _finalLitTexture;
    };
} // namespace Frost