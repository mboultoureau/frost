#include "Frost/Renderer/Pipeline/JoltDebugRenderingPipeline.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/InputLayout.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Debugging/DebugInterface/DebugPhysics.h"

#ifdef FT_PLATFORM_WINDOWS
#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/BufferDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#endif

#include <Jolt/Core/RTTI.h>

using namespace JPH;

namespace Frost
{
    struct alignas(16) VS_DebugPerFrameConstants
    {
        Math::Matrix4x4 ViewProjectionMatrix;
    };

    struct alignas(16) VS_DebugPerObjectConstants
    {
        Math::Matrix4x4 WorldMatrix;
    };

    JoltRenderingPipeline::JoltRenderingPipeline()
    {
        RendererAPI::GetRenderer()->RegisterPipeline(this);
        Initialize();
    }

    JoltRenderingPipeline::~JoltRenderingPipeline()
    {
        if (JPH::DebugRenderer::sInstance == this)
        {
            JPH::DebugRenderer::sInstance = nullptr;
        }
        RendererAPI::GetRenderer()->UnregisterPipeline(this);
        Shutdown();
    }

    void JoltRenderingPipeline::Initialize()
    {
#ifdef FT_PLATFORM_WINDOWS
        _commandList = std::make_unique<CommandListDX11>();
#else
#error "JoltDebugRendering only supports Windows platform in this implementation."
#endif

        // Shaders for lines
        ShaderDesc lineVSDesc = { .type = ShaderType::Vertex, .debugName = "VS_DebugLine", .filePath = "../Frost/resources/shaders/VS_DebugLine.hlsl" };
        ShaderDesc linePSDesc = { .type = ShaderType::Pixel, .debugName = "PS_DebugLine", .filePath = "../Frost/resources/shaders/PS_DebugLine.hlsl" };
        _debugLineVertexShader = std::make_unique<ShaderDX11>(lineVSDesc);
        _debugLinePixelShader = std::make_unique<ShaderDX11>(linePSDesc);

        const uint32_t lineVertexStride = sizeof(DebugLineVertex);
        InputLayout::VertexAttributeArray lineAttributes = {
            {.name = "POSITION", .format = Format::RGB32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = 0,  .elementStride = lineVertexStride, .isInstanced = false },
            {.name = "COLOR",    .format = Format::RGBA32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = sizeof(Math::Vector3), .elementStride = lineVertexStride, .isInstanced = false },
        };
        _debugLineInputLayout = std::make_unique<InputLayoutDX11>(lineAttributes, *_debugLineVertexShader);

        // Shaders for triangles
        ShaderDesc triVSDesc = { .type = ShaderType::Vertex, .debugName = "VS_DebugTriangle", .filePath = "../Frost/resources/shaders/VS_DebugTriangle.hlsl" };
        ShaderDesc triPSDesc = { .type = ShaderType::Pixel, .debugName = "PS_DebugTriangle", .filePath = "../Frost/resources/shaders/PS_DebugTriangle.hlsl" };
        _debugTriangleVertexShader = std::make_unique<ShaderDX11>(triVSDesc);
        _debugTrianglePixelShader = std::make_unique<ShaderDX11>(triPSDesc);

        const uint32_t triangleVertexStride = sizeof(DebugTriangleVertex);
        InputLayout::VertexAttributeArray triangleAttributes = {
            {.name = "POSITION", .format = Format::RGB32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = 0,  .elementStride = triangleVertexStride, .isInstanced = false },
            {.name = "NORMAL",   .format = Format::RGB32_FLOAT,  .arraySize = 1, .bufferIndex = 0, .offset = sizeof(Math::Vector3), .elementStride = triangleVertexStride, .isInstanced = false },
            {.name = "TEXCOORD", .format = Format::RG32_FLOAT,   .arraySize = 1, .bufferIndex = 0, .offset = sizeof(Math::Vector3) * 2, .elementStride = triangleVertexStride, .isInstanced = false },
            {.name = "COLOR",    .format = Format::RGBA32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = sizeof(Math::Vector3) * 2 + sizeof(Math::Vector2), .elementStride = triangleVertexStride, .isInstanced = false },
        };
        _debugTriangleInputLayout = std::make_unique<InputLayoutDX11>(triangleAttributes, *_debugTriangleVertexShader);

        // Constant Buffers
        auto* renderer = RendererAPI::GetRenderer();
        _vsPerFrameConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(VS_DebugPerFrameConstants), .dynamic = true });
        _vsPerObjectConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(VS_DebugPerObjectConstants), .dynamic = true });

        JPH::DebugRenderer::Initialize();
    }

    void JoltRenderingPipeline::Shutdown()
    {
        _vsPerObjectConstants.reset();
        _vsPerFrameConstants.reset();

        _debugTriangleInputLayout.reset();
        _debugTrianglePixelShader.reset();
        _debugTriangleVertexShader.reset();

        _debugLineInputLayout.reset();
        _debugLinePixelShader.reset();
        _debugLineVertexShader.reset();

        _lineVertexBuffer.reset();
        _triangleVertexBuffer.reset();

        _geometryBatches.clear();

        _commandList.reset();

        ClearFrameData();
    }

    Math::Vector4 JoltRenderingPipeline::ConvertJoltColor(JPH::ColorArg color)
    {
        return Math::Vector4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }

    void JoltRenderingPipeline::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
    {
        JPH::lock_guard lock(mLinesLock);
        _frameLines.push_back({ {inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()}, ConvertJoltColor(inColor) });
        _frameLines.push_back({ {inTo.GetX(), inTo.GetY(), inTo.GetZ()}, ConvertJoltColor(inColor) });
    }

    void JoltRenderingPipeline::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
    {
        JPH::lock_guard lock(mTrianglesLock);
        JPH::Vec3 normal = (inV2 - inV1).Cross(inV3 - inV1).Normalized();
        Math::Vector3 frostNormal = { normal.GetX(), normal.GetY(), normal.GetZ() };
        Math::Vector4 frostColor = ConvertJoltColor(inColor);

        _frameTriangles.push_back({ {inV1.GetX(), inV1.GetY(), inV1.GetZ()}, frostNormal, {0.0f, 0.0f}, frostColor });
        _frameTriangles.push_back({ {inV2.GetX(), inV2.GetY(), inV2.GetZ()}, frostNormal, {0.0f, 0.0f}, frostColor });
        _frameTriangles.push_back({ {inV3.GetX(), inV3.GetY(), inV3.GetZ()}, frostNormal, {0.0f, 0.0f}, frostColor });
    }

    JPH::DebugRenderer::Batch JoltRenderingPipeline::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount)
    {
        if (inTriangles == nullptr || inTriangleCount == 0)
        {
            return nullptr;
        }

        JPH::Ref<FrostTriangleBatch> batch = JPH::Ref<FrostTriangleBatch>(new FrostTriangleBatch());
        batch->vertexCount = inTriangleCount * 3;
        batch->indexCount = 0;

        std::vector<DebugTriangleVertex> vertices;
        vertices.reserve(batch->vertexCount);

        for (int i = 0; i < inTriangleCount; ++i)
        {
            const JPH::DebugRenderer::Triangle& tri = inTriangles[i];
            JPH::Vec3 p0 = JPH::Vec3(tri.mV[0].mPosition.x, tri.mV[0].mPosition.y, tri.mV[0].mPosition.z);
            JPH::Vec3 p1 = JPH::Vec3(tri.mV[1].mPosition.x, tri.mV[1].mPosition.y, tri.mV[1].mPosition.z);
            JPH::Vec3 p2 = JPH::Vec3(tri.mV[2].mPosition.x, tri.mV[2].mPosition.y, tri.mV[2].mPosition.z);
            JPH::Vec3 normal = (p1 - p0).Cross(p2 - p0).Normalized();
            Math::Vector3 frostNormal = { normal.GetX(), normal.GetY(), normal.GetZ() };
            Math::Vector4 frostColor = ConvertJoltColor(tri.mV[0].mColor);

            for (int v = 0; v < 3; ++v)
            {
                const JPH::DebugRenderer::Vertex& joltVtx = tri.mV[v];
                vertices.push_back({
                    {joltVtx.mPosition.x, joltVtx.mPosition.y, joltVtx.mPosition.z},
                    frostNormal,
                    {joltVtx.mUV.x, joltVtx.mUV.y},
                    ConvertJoltColor(joltVtx.mColor)
                    });
            }
        }

        batch->vertexBuffer = RendererAPI::GetRenderer()->CreateBuffer(BufferConfig{
            .usage = BufferUsage::VERTEX_BUFFER,
            .size = (uint32_t)(vertices.size() * sizeof(DebugTriangleVertex)),
            .dynamic = false,
            }, vertices.data());

        return JPH::StaticCast<JPH::RefTargetVirtual>(batch);
    }

    JPH::DebugRenderer::Batch JoltRenderingPipeline::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount)
    {
        if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
        {
            return nullptr;
        }

        JPH::Ref<FrostTriangleBatch> batch = JPH::Ref<FrostTriangleBatch>(new FrostTriangleBatch());
        batch->vertexCount = inVertexCount;
        batch->indexCount = inIndexCount;

        std::vector<DebugTriangleVertex> vertices;
        vertices.reserve(inVertexCount);
        for (int i = 0; i < inVertexCount; ++i)
        {
            const JPH::DebugRenderer::Vertex& joltVtx = inVertices[i];
            vertices.push_back({
                {joltVtx.mPosition.x, joltVtx.mPosition.y, joltVtx.mPosition.z},
                {joltVtx.mNormal.x, joltVtx.mNormal.y, joltVtx.mNormal.z},
                {joltVtx.mUV.x, joltVtx.mUV.y},
                ConvertJoltColor(joltVtx.mColor)
            });
        }

        batch->vertexBuffer = RendererAPI::GetRenderer()->CreateBuffer(BufferConfig{
            .usage = BufferUsage::VERTEX_BUFFER,
            .size = (uint32_t)(vertices.size() * sizeof(DebugTriangleVertex)),
            .dynamic = false,
        }, vertices.data());

        batch->indexBuffer = RendererAPI::GetRenderer()->CreateBuffer(BufferConfig{
            .usage = BufferUsage::INDEX_BUFFER,
            .size = (uint32_t)(inIndexCount * sizeof(uint32)),
            .dynamic = false,
        }, inIndices);

        return JPH::StaticCast<JPH::RefTargetVirtual>(batch);
    }

    void JoltRenderingPipeline::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const JPH::DebugRenderer::GeometryRef& inGeometry, JPH::DebugRenderer::ECullMode inCullMode, JPH::DebugRenderer::ECastShadow inCastShadow, JPH::DebugRenderer::EDrawMode inDrawMode)
    {
        if (!inGeometry || inGeometry->mLODs.empty())
            return;

        JPH::Ref<FrostTriangleBatch> batch = JPH::StaticCast<FrostTriangleBatch>(inGeometry->mLODs[0].mTriangleBatch);
        if (!batch || !batch->vertexBuffer)
            return;

        JPH::lock_guard lock(mGeometryCommandsLock);
        GeometryDrawCommand cmd;
        std::array<float, 16> joltMatrixElements;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                joltMatrixElements[r * 4 + c] = inModelMatrix(r, c);
            }
        }
        cmd.WorldMatrix = Math::Matrix4x4(joltMatrixElements);
        cmd.Batch = batch;
        cmd.ModelColor = ConvertJoltColor(inModelColor);
        _geometryDrawCommands.push_back(cmd);
    }

    void JoltRenderingPipeline::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
    {
        JPH::lock_guard lock(mTextsLock);
        _frameTexts.push_back({ inPosition, std::string(inString), inColor, inHeight });
    }

    void JoltRenderingPipeline::BeginFrame(const Component::Camera& camera, const Component::WorldTransform& cameraTransform)
    {
        _commandList->BeginRecording();
		_commandList->SetRasterizerState(RasterizerMode::Wireframe);

        float aspectRatio = static_cast<float>(Application::GetWindow()->GetWidth()) / static_cast<float>(Application::GetWindow()->GetHeight());
        Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
        Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, aspectRatio);

        VS_DebugPerFrameConstants vsPerFrameData;
        vsPerFrameData.ViewProjectionMatrix = Math::Matrix4x4::CreateTranspose(viewMatrix * projectionMatrix);
        _vsPerFrameConstants->UpdateData(_commandList.get(), &vsPerFrameData, sizeof(VS_DebugPerFrameConstants));
        _commandList->SetConstantBuffer(_vsPerFrameConstants.get(), 0);

        Texture* backBuffer = RendererAPI::GetRenderer()->GetBackBuffer();
        Texture* depthBuffer = RendererAPI::GetRenderer()->GetDepthBuffer();
        _commandList->SetRenderTargets(1, &backBuffer, depthBuffer);

        _commandList->SetViewport(0.0f, 0.0f, (float)Application::GetWindow()->GetWidth(), (float)Application::GetWindow()->GetHeight(), 0.0f, 1.0f);
    }

    void JoltRenderingPipeline::EndFrame()
    {
        // Draw accumulated lines
        if (!_frameLines.empty())
        {
            if (!_lineVertexBuffer || _lineVertexBuffer->GetSize() < _frameLines.size() * sizeof(DebugLineVertex))
            {
                _lineVertexBuffer = RendererAPI::GetRenderer()->CreateBuffer(BufferConfig{
                    .usage = BufferUsage::VERTEX_BUFFER,
                    .size = (uint32_t)(_frameLines.size() * sizeof(DebugLineVertex)),
                    .dynamic = true
                });
            }
            _lineVertexBuffer->UpdateData(_commandList.get(), _frameLines.data(), (uint32_t)(_frameLines.size() * sizeof(DebugLineVertex)));

            _commandList->SetShader(_debugLineVertexShader.get());
            _commandList->SetShader(_debugLinePixelShader.get());
            _commandList->SetInputLayout(_debugLineInputLayout.get());
            _commandList->SetPrimitiveTopology(PrimitiveTopology::LINELIST);
            _commandList->SetVertexBuffer(_lineVertexBuffer.get(), sizeof(DebugLineVertex), 0);
            _commandList->Draw((uint32_t)_frameLines.size(), 0);
        }

        // Draw accumulated triangles
        if (!_frameTriangles.empty())
        {
            if (!_triangleVertexBuffer || _triangleVertexBuffer->GetSize() < _frameTriangles.size() * sizeof(DebugTriangleVertex))
            {
                _triangleVertexBuffer = RendererAPI::GetRenderer()->CreateBuffer(BufferConfig{
                    .usage = BufferUsage::VERTEX_BUFFER,
                    .size = (uint32_t)(_frameTriangles.size() * sizeof(DebugTriangleVertex)),
                    .dynamic = true
                });
            }
            _triangleVertexBuffer->UpdateData(_commandList.get(), _frameTriangles.data(), (uint32_t)(_frameTriangles.size() * sizeof(DebugTriangleVertex)));

            _commandList->SetShader(_debugTriangleVertexShader.get());
            _commandList->SetShader(_debugTrianglePixelShader.get());
            _commandList->SetInputLayout(_debugTriangleInputLayout.get());
            _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
            _commandList->SetVertexBuffer(_triangleVertexBuffer.get(), sizeof(DebugTriangleVertex), 0);
            _commandList->Draw((uint32_t)_frameTriangles.size(), 0);
        }

        for (const auto& cmd : _geometryDrawCommands)
        {
            VS_DebugPerObjectConstants perObjectData;
            perObjectData.WorldMatrix = cmd.WorldMatrix;
            _vsPerObjectConstants->UpdateData(_commandList.get(), &perObjectData, sizeof(VS_DebugPerObjectConstants));
            _commandList->SetConstantBuffer(_vsPerObjectConstants.get(), 1);

            _commandList->SetShader(_debugTriangleVertexShader.get());
            _commandList->SetShader(_debugTrianglePixelShader.get());
            _commandList->SetInputLayout(_debugTriangleInputLayout.get());
            _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

            _commandList->SetVertexBuffer(cmd.Batch->vertexBuffer.get(), sizeof(DebugTriangleVertex), 0);
            if (cmd.Batch->indexBuffer)
            {
                _commandList->SetIndexBuffer(cmd.Batch->indexBuffer.get(), 0);
                _commandList->DrawIndexed(cmd.Batch->indexCount, 0, 0);
            }
            else
            {
                _commandList->Draw(cmd.Batch->vertexCount, 0);
            }
        }

        for (const auto& text : _frameTexts)
        {
        }

        _commandList->SetRasterizerState(RasterizerMode::Solid);

        _commandList->EndRecording();

#ifdef FT_DEBUG
        if (Debug::PhysicsConfig::display)
        {
            _commandList->Execute();
        }
#endif

        RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();

        ClearFrameData();
    }

    void JoltRenderingPipeline::ClearFrameData()
    {
        JPH::lock_guard lockLines(mLinesLock);
        _frameLines.clear();

        JPH::lock_guard lockTriangles(mTrianglesLock);
        _frameTriangles.clear();

        JPH::lock_guard lockTexts(mTextsLock);
        _frameTexts.clear();

        JPH::lock_guard lockGeometry(mGeometryCommandsLock);
        _geometryDrawCommands.clear();

        JPH::DebugRenderer::NextFrame();
    }
}