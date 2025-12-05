#include "Editor/UI/Scene/SceneView.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/Math/Transform.h"

namespace Editor
{
    using namespace Frost;
    using namespace Frost::Math;
    using namespace Frost::Component;

    SceneView::SceneView(const std::string& title, Frost::Scene* existingScene) :
        _title(title),
        _sceneContext(existingScene),
        _isPrefabView(false),
        _gizmo{ std::make_unique<Gizmo>(_sceneContext) }
    {
        _Init();
    }

    SceneView::SceneView(const std::filesystem::path& prefabPath) : _assetPath(prefabPath), _isPrefabView(true)
    {
        _title = prefabPath.stem().string();
        _localScene = std::make_unique<Frost::Scene>("Prefab Editor");
        _sceneContext = _localScene.get();
        _gizmo = std::make_unique<Gizmo>(_sceneContext);

        _Init();

        if (std::filesystem::exists(_assetPath))
        {
            Frost::PrefabSerializer::Instantiate(_sceneContext, _assetPath);

            Frost::BoundingBox totalBounds;
            bool hasBounds = false;

            auto meshView = _sceneContext->ViewActive<Transform, StaticMesh>();

            meshView.each(
                [&](const auto& transform, const auto& mesh)
                {
                    if (mesh.GetModel() && mesh.GetModel()->HasMeshes())
                    {
                        Frost::BoundingBox localBounds = mesh.GetModel()->GetBoundingBox();
                        Frost::BoundingBox worldBounds =
                            Frost::Math::TransformBoundingBox(localBounds, transform.GetTransformMatrix());

                        if (!hasBounds)
                        {
                            totalBounds = worldBounds;
                            hasBounds = true;
                        }
                        else
                        {
                            totalBounds.Merge(worldBounds);
                        }
                    }
                });

            if (hasBounds)
            {
                auto& camTrans = _editorCamera.GetComponent<Transform>();
                _FocusCameraOnEntity(camTrans, totalBounds);
            }
        }
    }

    SceneView::SceneView(const std::filesystem::path& meshPath, MeshPreviewTag)
    {
        _title = "Preview: " + meshPath.filename().string();
        _localScene = std::make_unique<Frost::Scene>("Mesh Preview");
        _sceneContext = _localScene.get();
        _gizmo = std::make_unique<Gizmo>(_sceneContext);

        _Init();

        auto meshEntity = _sceneContext->CreateGameObject(meshPath.stem().string());
        meshEntity.AddComponent<Transform>(
            Vector3{ 0.0f, 0.0f, 0.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
        auto& staticMesh = meshEntity.AddComponent<Frost::Component::StaticMesh>(MeshSourceFile{ meshPath.string() });

        if (!staticMesh.GetModel() || !staticMesh.GetModel()->HasMeshes())
        {
            FT_ENGINE_ERROR("Failed to load model for mesh preview: {}", meshPath.string());
            return;
        }

        BoundingBox bounds = staticMesh.GetModel()->GetBoundingBox();
        auto& camTrans = _editorCamera.GetComponent<Transform>();
        _FocusCameraOnEntity(camTrans, bounds);
    }

    void SceneView::_Init()
    {
        _ResizeViewportFramebuffer(1280, 720);

        _editorCamera = _sceneContext->CreateGameObject("__EDITOR__Camera");
        auto& tc = _editorCamera.AddComponent<Transform>();
        tc.position = { 0.0f, 2.0f, -10.0f };
        auto& cam = _editorCamera.AddComponent<Camera>();
        cam.priority = 100;
        cam.viewport = { 0.0f, 0.0f, 1.0f, 1.0f };
        cam.farClip = 1000.0f;
        cam.nearClip = 0.1f;

        _editorSkybox = _sceneContext->CreateGameObject("__EDITOR__Skybox");
        _editorSkybox.AddComponent<Skybox>("./resources/editor/skyboxes/Cubemap_Sky_04-512x512.png");

        _editorLight = _sceneContext->CreateGameObject("__EDITOR__DirectionalLight");
        auto& lightTransform = _editorLight.AddComponent<Transform>();
        _editorLight.AddComponent<Light>();
        lightTransform.position = { 0.0f, 5.0f, -5.0f };
        lightTransform.Rotate(EulerAngles{ -45.0f, 45.0f, 0.0f });

        _cameraController.Initialize(tc);
        _toolbar.Init();
    }

    void SceneView::OnUpdate(float deltaTime)
    {
        if (_sceneContext && _viewportTexture)
        {
            _sceneContext->SetEditorRenderTarget(_viewportTexture);
            _sceneContext->Update(deltaTime);
            _sceneContext->LateUpdate(deltaTime);
        }
    }

    void SceneView::OnFixedUpdate(float deltaTime)
    {
        if (_sceneContext && _viewportTexture)
        {
            _sceneContext->PreFixedUpdate(deltaTime);
            _sceneContext->FixedUpdate(deltaTime);
        }
    }

    void SceneView::_ResizeViewportFramebuffer(uint32_t width, uint32_t height)
    {
        if (_viewportTexture && _viewportTexture->GetWidth() == width && _viewportTexture->GetHeight() == height)
            return;

        _viewportWidth = width;
        _viewportHeight = height;

        Frost::TextureConfig config;
        config.width = width;
        config.height = height;
        config.format = Frost::Format::RGBA8_UNORM;
        config.isRenderTarget = true;
        config.isShaderResource = true;
        config.debugName = "Editor_SceneViewport";
        _viewportTexture = Frost::Texture::Create(config);
    }
} // namespace Editor