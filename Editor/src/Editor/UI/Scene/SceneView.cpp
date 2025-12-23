#include "Editor/UI/Scene/SceneView.h"

#include "Frost/Scene/Components/EnvironmentMap.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Scene/SceneSerializer.h"
#include "Frost/Debugging/Logger.h"

namespace Editor
{
    using namespace Frost;
    using namespace Frost::Math;
    using namespace Frost::Component;

    bool SceneView::_IsSelected(const Frost::GameObject& go) const
    {
        for (const auto& item : _selection)
        {
            if (item.GetHandle() == go.GetHandle())
                return true;
        }
        return false;
    }

    void SceneView::_AddToSelection(const Frost::GameObject& go)
    {
        if (!_IsSelected(go))
            _selection.push_back(go);
    }

    void SceneView::_RemoveFromSelection(const Frost::GameObject& go)
    {
        _selection.erase(std::remove_if(_selection.begin(),
                                        _selection.end(),
                                        [&](const Frost::GameObject& item)
                                        { return item.GetHandle() == go.GetHandle(); }),
                         _selection.end());
    }

    void SceneView::_ClearSelection()
    {
        _selection.clear();
    }

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

    SceneView::SceneView(const std::filesystem::path& scenePath, SceneTag) : _assetPath(scenePath), _isPrefabView(false)
    {
        _title = "Scene: " + scenePath.stem().string();
        _localScene = std::make_unique<Frost::Scene>("Scene Editor");
        _sceneContext = _localScene.get();
        _gizmo = std::make_unique<Gizmo>(_sceneContext);

        if (std::filesystem::exists(_assetPath))
        {
            SceneSerializer serializer(_sceneContext);
            if (!serializer.Deserialize(_assetPath))
            {
                FT_ENGINE_ERROR("Failed to load scene for editing: {0}", _assetPath.string());
            }
        }

        _Init();
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

        _editorCamera.AddComponent<Skybox>(
            SkyboxSourceCubemap{ "./resources/editor/skyboxes/Cubemap_Sky_04-512x512.png" });

        _editorLight = _sceneContext->CreateGameObject("__EDITOR__AmbientLight");
        _editorLight.AddComponent<Light>(LightAmbient{});

        _editorEnvironment = _sceneContext->CreateGameObject("__EDITOR__Environment");
        auto& env = _editorEnvironment.AddComponent<EnvironmentMap>(
            EnvironmentMapSourceCubemap{ "./resources/editor/skyboxes/Cubemap_Sky_04-512x512.png" });
        env.intensity = 1.0f;

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