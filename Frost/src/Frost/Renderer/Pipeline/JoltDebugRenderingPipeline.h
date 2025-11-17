#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/Pipeline.h"
#include "Frost/Utils/Math/Matrix.h"


#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.h>
#else
#error "JPH_DEBUG_RENDERER must be defined to use JoltDebugRendering."
#endif

#include <Jolt/Core/Core.h>
#include <Jolt/Core/Mutex.h>
#include <Jolt/Core/UnorderedMap.h>

#include <memory>
#include <vector>
#include <atomic>

namespace Frost
{
    class CommandList;
    class Buffer;
    class Shader;
    class InputLayout;
    class Sampler;
    class Texture;

    struct DebugLineVertex
    {
        Math::Vector3 position;
        Math::Vector4 color;
    };

    struct DebugTriangleVertex
    {
        Math::Vector3 position;
        Math::Vector3 normal;
        Math::Vector2 uv;
        Math::Vector4 color;
    };

    class JoltRenderingPipeline final : public Pipeline, public JPH::DebugRenderer
    {
    public:
        JoltRenderingPipeline();
        ~JoltRenderingPipeline();

        void Initialize() override;
        void Shutdown() override;

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount) override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;
        void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const JPH::DebugRenderer::GeometryRef& inGeometry, JPH::DebugRenderer::ECullMode inCullMode, JPH::DebugRenderer::ECastShadow inCastShadow, JPH::DebugRenderer::EDrawMode inDrawMode) override;
        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override;

        void BeginFrame(const Component::Camera& camera, const Component::WorldTransform& cameraTransform);
        void EndFrame();
        void ClearFrameData();

    private:
        std::unique_ptr<CommandList> _commandList;

        std::unique_ptr<Shader> _debugLineVertexShader;
        std::unique_ptr<Shader> _debugLinePixelShader;
        std::unique_ptr<InputLayout> _debugLineInputLayout;

        std::unique_ptr<Shader> _debugTriangleVertexShader;
        std::unique_ptr<Shader> _debugTrianglePixelShader;
        std::unique_ptr<InputLayout> _debugTriangleInputLayout;

        // Constant Buffers
        std::unique_ptr<Buffer> _vsPerFrameConstants;
        std::unique_ptr<Buffer> _vsPerObjectConstants;

        // Data to draw this frame
        std::vector<DebugLineVertex> _frameLines;
        std::vector<DebugTriangleVertex> _frameTriangles;

        struct DebugText3D
        {
            JPH::RVec3 position;
            std::string text;
            JPH::Color color;
            float height;
        };
        std::vector<DebugText3D> _frameTexts;

        // Buffers for lines and triangles
        std::unique_ptr<Buffer> _lineVertexBuffer;
        std::unique_ptr<Buffer> _triangleVertexBuffer;

        // Mutexes for thread-safe access to frame data
        JPH::Mutex mLinesLock;
        JPH::Mutex mTrianglesLock;
        JPH::Mutex mTextsLock;
        JPH::Mutex mGeometryCommandsLock;

        class FrostTriangleBatch : public JPH::RefTargetVirtual
        {
        public:
            virtual ~FrostTriangleBatch() override = default;
            void AddRef() override { mRefCount++; }
            void Release() override { if (--mRefCount == 0) delete this; }

            std::unique_ptr<Buffer> vertexBuffer;
            std::unique_ptr<Buffer> indexBuffer;
            JPH::uint32 vertexCount;
            JPH::uint32 indexCount;

        private:
            std::atomic<int> mRefCount{ 0 };
        };

        struct GeometryDrawCommand
        {
            Math::Matrix4x4 WorldMatrix;
            JPH::Ref<FrostTriangleBatch> Batch;
            Math::Vector4 ModelColor;
        };
        std::vector<GeometryDrawCommand> _geometryDrawCommands;

        JPH::UnorderedMap<const JPH::DebugRenderer::Geometry*, std::vector<JPH::Ref<FrostTriangleBatch>>> _geometryBatches;
        Math::Vector4 ConvertJoltColor(JPH::ColorArg color);
    };
}