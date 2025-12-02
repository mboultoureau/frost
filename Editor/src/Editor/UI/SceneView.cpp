#include "Editor/UI/SceneView.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Disabled.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Debugging/ComponentUIRegistry.h"
#include "Frost/Asset/MeshConfig.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Utils/Math/Matrix.h"

#undef max

#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
    using namespace Frost;
    using namespace Frost::Math;
    using namespace Frost::Component;

    SceneView::SceneView(const std::string& title, Frost::Scene* existingScene) :
        _title(title), _sceneContext(existingScene), _isPrefabView(false)
    {
        _Init();
    }

    SceneView::SceneView(const std::filesystem::path& prefabPath) : _assetPath(prefabPath), _isPrefabView(true)
    {
        _title = prefabPath.stem().string();
        _localScene = std::make_unique<Frost::Scene>("Prefab Editor");
        _sceneContext = _localScene.get();

        if (std::filesystem::exists(_assetPath))
        {
            Frost::PrefabSerializer::Instantiate(_sceneContext, _assetPath);
        }

        _Init();
    }

    SceneView::SceneView(const std::filesystem::path& meshPath, MeshPreviewTag)
    {
        _title = "Preview: " + meshPath.filename().string();

        _localScene = std::make_unique<Frost::Scene>("Mesh Preview");
        _sceneContext = _localScene.get();

        _Init();

        auto meshEntity = _sceneContext->CreateGameObject(meshPath.stem().string());

        meshEntity.AddComponent<Transform>(
            Vector3{ 0.0f, 0.0f, 0.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });

        auto& staticMesh = meshEntity.AddComponent<Frost::Component::StaticMesh>(MeshSourceFile{ meshPath.string() });
        if (!staticMesh.GetModel())
        {
            FT_ENGINE_ERROR("Failed to load model for mesh preview: {}", meshPath.string());
            return;
        }

        if (staticMesh.GetModel()->HasMeshes())
        {
            BoundingBox bounds = staticMesh.GetModel()->GetBoundingBox();
            auto& camTrans = _editorCamera.GetComponent<Transform>();
            _FocusCameraOnEntity(camTrans, bounds);
        }
    }

    void SceneView::_FocusCameraOnEntity(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds)
    {
        using namespace DirectX;

        if (bounds.min.x >= bounds.max.x)
        {
            return;
        }

        XMFLOAT3 center = { (bounds.min.x + bounds.max.x) * 0.5f,
                            (bounds.min.y + bounds.max.y) * 0.5f,
                            (bounds.min.z + bounds.max.z) * 0.5f };

        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxDim = std::max({ sizeX, sizeY, sizeZ });

        if (maxDim < 0.1f)
        {
            maxDim = 0.1f;
        }

        float distance = maxDim * 1.5f;

        cameraTransform.position =
            Frost::Math::Vector3{ center.x + distance * 0.5f, center.y + distance * 0.5f, center.z - distance };

        cameraTransform.LookAt(Frost::Math::Vector3{ center.x, center.y, center.z });
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

        auto& skybox = _editorCamera.AddComponent<Skybox>("./resources/editor/skyboxes/Cubemap_Sky_04-512x512.png");

        auto light = _sceneContext->CreateGameObject("__EDITOR__DirectionalLight");
        auto& lightTransform = light.AddComponent<Transform>();
        auto& lightComponent = light.AddComponent<Light>();

        lightTransform.position = { 0.0f, 5.0f, -5.0f };
        lightTransform.Rotate(EulerAngles{ -45.0f, 45.0f, 0.0f });

        lightComponent.type = LightType::Directional;
        lightComponent.color = { 1.0f, 1.0f, 1.0f };
        lightComponent.intensity = 1.0f;

        _cameraController.Initialize(tc);
    }

    void SceneView::OnUpdate(float deltaTime)
    {
        if (_editorCamera && _editorCamera.HasComponent<Transform>())
        {
            auto& tc = _editorCamera.GetComponent<Transform>();
            bool canControl = _isHovered;

            _cameraController.OnUpdate(deltaTime, tc, canControl);
        }

        if (_sceneContext && _viewportTexture)
        {
            _sceneContext->SetEditorRenderTarget(_viewportTexture);
            _sceneContext->Update(deltaTime);
            _sceneContext->LateUpdate(deltaTime);
        }
    }

    void SceneView::Draw()
    {
        DrawViewport();
    }

    void SceneView::_ResizeViewportFramebuffer(uint32_t width, uint32_t height)
    {
        if (_viewportTexture && _viewportTexture->GetWidth() == width && _viewportTexture->GetHeight() == height)
            return;

        _viewportWidth = width;
        _viewportHeight = height;

        Frost::TextureConfig config = {};
        config.width = width;
        config.height = height;
        config.format = Frost::Format::RGBA8_UNORM;
        config.isRenderTarget = true;
        config.isShaderResource = true;
        config.hasMipmaps = false;
        config.debugName = "Editor_SceneViewport";

        _viewportTexture = Frost::Texture::Create(config);
    }

    void SceneView::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        bool open = true;
        ImGui::Begin(_title.c_str(), &open);
        _isOpen = open;

        _isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        _isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        _DrawToolbar();

        ImVec2 viewportMinRegion = ImGui::GetCursorScreenPos();
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        viewportPanelSize.x = std::max(viewportPanelSize.x, 1.0f);
        viewportPanelSize.y = std::max(viewportPanelSize.y, 1.0f);

        _ResizeViewportFramebuffer((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);

        if (_viewportTexture)
        {
            ImTextureID textureID = (ImTextureID)_viewportTexture->GetRendererID();
            ImGui::Image(textureID, viewportPanelSize);
        }

        // Drop Target
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::filesystem::path assetPath = std::filesystem::path(path);

                std::string ext = assetPath.extension().string();
                if (std::find(MESH_FILE_EXTENSIONS.begin(), MESH_FILE_EXTENSIONS.end(), ext) !=
                    MESH_FILE_EXTENSIONS.end())
                {
                    _HandleMeshDrop(assetPath);
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void SceneView::_HandleMeshDrop(const std::filesystem::path& relativePath)
    {
        if (!_sceneContext)
        {
            return;
        }

        Frost::Math::Vector3 spawnPos = _GetSpawnPositionFromMouse();

        std::string name = relativePath.stem().string();
        auto newEntity = _sceneContext->CreateGameObject(name);

        newEntity.AddComponent<Transform>(spawnPos, EulerAngles{ 0, 0, 0 }, Vector3{ 1, 1, 1 });
        newEntity.AddComponent<Frost::Component::StaticMesh>(Frost::Component::MeshSourceFile{ relativePath.string() });

        if (_isPrefabView)
        {
            auto view = _sceneContext->GetRegistry().view<Relationship>();
            for (auto entity : view)
            {
                if (entity == (entt::entity)newEntity)
                {
                    continue;
                }

                const auto& rel = view.get<Relationship>(entity);

                if (rel.parent == entt::null)
                {
                    auto* meta = _sceneContext->GetRegistry().try_get<Meta>(entity);
                    if (meta && meta->name.find("__EDITOR__") == 0)
                    {
                        continue;
                    }

                    newEntity.SetParent(Frost::GameObject(entity, _sceneContext));
                    break;
                }
            }
        }

        _selection = newEntity;
    }

    Frost::Math::Vector3 SceneView::_GetSpawnPositionFromMouse()
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();

        ImVec2 min = ImGui::GetWindowContentRegionMin();
        ImVec2 max = ImGui::GetWindowContentRegionMax();

        min.x += windowPos.x;
        min.y += windowPos.y;
        max.x += windowPos.x;
        max.y += windowPos.y;

        float viewportW = max.x - min.x;
        float viewportH = max.y - min.y;

        float mx = mousePos.x - min.x;
        float my = mousePos.y - min.y;

        auto [rayOrigin, rayDir] = _GetCameraRay(mx, my, viewportW, viewportH);

        Frost::Math::Vector3 planeNormal = { 0.0f, 1.0f, 0.0f }; // Up vector
        Frost::Math::Vector3 planePoint = { 0.0f, 0.0f, 0.0f };  // Center of world

        float denominator = Frost::Math::Dot(rayDir, planeNormal);

        if (std::abs(denominator) > 0.0001f)
        {
            Frost::Math::Vector3 p0_min_origin = planePoint - rayOrigin;
            float t = Frost::Math::Dot(p0_min_origin, planeNormal) / denominator;

            if (t >= 0.0f)
            {
                if (t < 1000.0f)
                {
                    return rayOrigin + (rayDir * t);
                }
            }
        }

        float defaultDistance = 10.0f;
        return rayOrigin + (rayDir * defaultDistance);
    }

    std::pair<Frost::Math::Vector3, Frost::Math::Vector3> SceneView::_GetCameraRay(float mouseX,
                                                                                   float mouseY,
                                                                                   float viewportW,
                                                                                   float viewportH)
    {
        using namespace Frost::Math;

        float ndcX = (2.0f * mouseX) / viewportW - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY) / viewportH;

        auto& camTrans = _editorCamera.GetComponent<Frost::Component::Transform>();
        auto& camComp = _editorCamera.GetComponent<Frost::Component::Camera>();

        Matrix4x4 rotMat = Matrix4x4::CreateFromQuaternion(camTrans.rotation);
        Matrix4x4 transMat = Matrix4x4::CreateTranslation(camTrans.position);

        Matrix4x4 worldMat = rotMat * transMat;
        Matrix4x4 viewMat = Matrix4x4::Invert(worldMat);

        float aspectRatio = viewportW / viewportH;
        Matrix4x4 projMat;

        if (camComp.projectionType == Frost::Component::Camera::ProjectionType::Perspective)
        {
            projMat = Matrix4x4::CreatePerspectiveFovLH(
                camComp.perspectiveFOV.value(), aspectRatio, camComp.nearClip, camComp.farClip);
        }
        else
        {
            float width = camComp.orthographicSize * aspectRatio;
            float height = camComp.orthographicSize;
            projMat = Matrix4x4::CreateOrthographicLH(width, height, camComp.nearClip, camComp.farClip);
        }

        Matrix4x4 viewProj = viewMat * projMat;
        Matrix4x4 invViewProj = Matrix4x4::Invert(viewProj);

        Vector4 nearPointNDC = { ndcX, ndcY, 0.0f, 1.0f };
        Vector4 farPointNDC = { ndcX, ndcY, 1.0f, 1.0f };

        Vector4 nearPointWorld = TransformVector4(nearPointNDC, invViewProj);
        Vector4 farPointWorld = TransformVector4(farPointNDC, invViewProj);

        if (nearPointWorld.w != 0.0f)
        {
            nearPointWorld = nearPointWorld * (1.0f / nearPointWorld.w);
        }
        if (farPointWorld.w != 0.0f)
        {
            farPointWorld = farPointWorld * (1.0f / farPointWorld.w);
        }

        Vector3 rayOrigin = { nearPointWorld.x, nearPointWorld.y, nearPointWorld.z };
        Vector3 rayEnd = { farPointWorld.x, farPointWorld.y, farPointWorld.z };

        Vector3 rayDir = Normalize(rayEnd - rayOrigin);

        return { rayOrigin, rayDir };
    }

    void SceneView::OnRenderHierarchy()
    {
        entt::entity prefabRoot = entt::null;
        if (_isPrefabView)
        {
            auto view = _sceneContext->GetRegistry().view<Relationship>();
            int rootCount = 0;

            for (auto entity : view)
            {
                auto* meta = _sceneContext->GetRegistry().try_get<Meta>(entity);
                if (meta && meta->name.find("__EDITOR__") == 0)
                    continue;

                if (view.get<Relationship>(entity).parent == entt::null)
                {
                    prefabRoot = entity;
                    rootCount++;
                }
            }

            if (rootCount == 0)
            {
                auto newRoot = _sceneContext->CreateGameObject("Prefab Root");
                prefabRoot = (entt::entity)newRoot.GetId();
            }
        }

        if (!_isReadOnly)
        {
            if (ImGui::BeginPopupContextWindow("HierarchyContext", 1))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                {
                    auto newEntity = _sceneContext->CreateGameObject("Entity");

                    if (_isPrefabView && prefabRoot != entt::null)
                    {
                        newEntity.SetParent(Frost::GameObject(prefabRoot, _sceneContext));
                    }
                }
                ImGui::EndPopup();
            }
        }

        auto& registry = _sceneContext->GetRegistry();

        registry.view<Relationship>().each(
            [&](auto entityID, auto& relation)
            {
                if (relation.parent == entt::null)
                {
                    _DrawEntityNode(entityID);
                }
            });

        if (!_isReadOnly &&
            ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->Rect(), ImGui::GetID("HierarchyContent")))
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_NODE"))
            {
                entt::entity payloadEntity = *(entt::entity*)payload->Data;

                if (!_isPrefabView)
                {
                    _ReparentEntity(payloadEntity, entt::null);
                }
                else
                {
                    if (prefabRoot != entt::null && payloadEntity != prefabRoot)
                    {
                        _ReparentEntity(payloadEntity, prefabRoot);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        {
            _selection = {};
        }
    }

    void SceneView::_DrawEntityNode(entt::entity entityID)
    {
        auto& registry = _sceneContext->GetRegistry();
        if (!registry.valid(entityID))
            return;

        auto* meta = registry.try_get<Meta>(entityID);
        auto* relation = registry.try_get<Relationship>(entityID);
        bool isDisabled = registry.all_of<Disabled>(entityID);

        if (meta && meta->name.find("__EDITOR__") == 0)
            return;

        bool isPrefabRoot = false;
        if (_isPrefabView && relation && relation->parent == entt::null)
        {
            isPrefabRoot = true;
        }

        std::string name = meta ? meta->name : "Entity " + std::to_string((uint32_t)entityID);

        ImGuiTreeNodeFlags flags = ((_selection == (entt::entity)entityID) ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DrawLinesFull;

        if (relation && relation->childrenCount == 0)
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (isDisabled)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entityID, flags, name.c_str());

        if (isDisabled)
            ImGui::PopStyleColor();

        if (ImGui::IsItemClicked())
        {
            _selection = Frost::GameObject(entityID, _sceneContext);
        }

        if (!_isReadOnly && !isPrefabRoot && ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("HIERARCHY_NODE", &entityID, sizeof(entt::entity));
            ImGui::Text("%s", name.c_str());
            ImGui::EndDragDropSource();
        }

        if (!_isReadOnly && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_NODE"))
            {
                entt::entity payloadEntity = *(entt::entity*)payload->Data;
                if (payloadEntity != entityID)
                {
                    _ReparentEntity(payloadEntity, entityID);
                }
            }
            ImGui::EndDragDropTarget();
        }

        bool entityDeleted = false;
        if (!_isReadOnly && ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create Child"))
            {
                Frost::GameObject parentObj(entityID, _sceneContext);
                auto child = _sceneContext->CreateGameObject("New Entity");
                _ReparentEntity((entt::entity)child, entityID);
                ImGui::SetNextItemOpen(true);
            }

            if (!isPrefabRoot)
            {
                if (ImGui::MenuItem("Delete Entity"))
                {
                    entityDeleted = true;
                }
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::MenuItem("Delete Entity (Root Locked)");
                ImGui::PopStyleColor();
            }

            ImGui::EndPopup();
        }

        if (opened)
        {
            if (relation)
            {
                entt::entity currentChild = relation->firstChild;
                while (registry.valid(currentChild))
                {
                    _DrawEntityNode(currentChild);
                    auto* childRel = registry.try_get<Relationship>(currentChild);
                    currentChild = childRel ? childRel->nextSibling : entt::null;
                }
            }
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            _sceneContext->DestroyGameObject(Frost::GameObject(entityID, _sceneContext));
            if (_selection == (entt::entity)entityID)
            {
                _selection = {};
            }
        }
    }

    void SceneView::_ReparentEntity(entt::entity entityID, entt::entity newParentID)
    {
        Frost::GameObject obj(entityID, _sceneContext);

        Frost::GameObject parentObj;
        if (newParentID != entt::null)
        {
            parentObj = Frost::GameObject(newParentID, _sceneContext);
        }
        else
        {
            parentObj = Frost::GameObject::InvalidId;
        }

        obj.SetParent(parentObj);
    }

    void SceneView::OnRenderInspector()
    {
        if (!_selection)
        {
            ImGui::TextDisabled("Select an entity in '%s' to view properties.", _title.c_str());
            return;
        }

        Frost::UIContext ctx;
        ctx.isEditor = true;
        ctx.deltaTime = ImGui::GetIO().DeltaTime;

        Frost::ComponentUIRegistry::DrawAll(_sceneContext, (entt::entity)_selection, ctx);

        ImGui::Separator();

        float width = ImGui::GetContentRegionAvail().x;
        if (ImGui::Button("Add Component", ImVec2(width, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            static char searchBuffer[64] = "";
            ImGui::InputTextWithHint("##SearchComponent", "Search...", searchBuffer, sizeof(searchBuffer));
            ImGui::Separator();

            for (const auto& serializer : SerializationSystem::GetAllSerializers())
            {
                if (searchBuffer[0] != '\0' && serializer.Name.find(searchBuffer) == std::string::npos)
                {
                    continue;
                }

                if (serializer.HasComponent(_selection))
                {
                    continue;
                }

                if (serializer.Name == "Relationship" || serializer.Name == "Meta" ||
                    serializer.Name == "WorldTransform")
                {
                    continue;
                }

                if (ImGui::MenuItem(serializer.Name.c_str()))
                {
                    serializer.AddComponent(_selection);
                    ImGui::CloseCurrentPopup();

                    searchBuffer[0] = '\0';
                }
            }

            ImGui::EndPopup();
        }
    }

    void SceneView::_DrawToolbar()
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);

        if (ImGui::Button("Translate"))
        {
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate"))
        {
        }
        ImGui::SameLine();
        if (ImGui::Button("Scale"))
        {
        }

        if (_isPrefabView && !_isReadOnly)
        {
            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();
            if (ImGui::Button("Save Prefab"))
            {
                auto& registry = _sceneContext->GetRegistry();
                auto view = _sceneContext->GetRegistry().view<Relationship>();

                int rootCount = 0;
                entt::entity rootEntity = entt::null;

                for (auto entity : view)
                {
                    const auto& rel = view.get<Relationship>(entity);

                    if (rel.parent != entt::null)
                    {
                        continue;
                    }

                    if (auto* meta = registry.try_get<Meta>(entity))
                    {
                        if (meta->name.find("__EDITOR__") == 0)
                        {
                            continue;
                        }
                    }

                    rootCount++;
                    rootEntity = entity;
                }

                if (rootCount == 1)
                {
                    Frost::PrefabSerializer::CreatePrefab(Frost::GameObject(rootEntity, _sceneContext), _assetPath);
                    FT_ENGINE_INFO("Prefab saved successfully: {}", _assetPath.string());
                }
                else
                {
                    FT_ENGINE_ERROR("Cannot save Prefab: It must have exactly ONE root entity. Found: {}", rootCount);
                    ImGui::OpenPopup("SaveErrorPopup");
                }
            }

            if (ImGui::BeginPopup("SaveErrorPopup"))
            {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Save Failed!");
                ImGui::Separator();
                ImGui::Text("A Prefab must contain exactly one root object.");
                ImGui::Text("Please parent your objects under a single container.");
                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::Separator();
    }

    bool SceneView::_DrawVec3Control(const std::string& label, float* values, float resetValue, float columnWidth)
    {
        bool changed = false;
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        if (ImGui::Button("X", buttonSize))
        {
            values[0] = resetValue;
            changed = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
            changed = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (ImGui::Button("Y", buttonSize))
        {
            values[1] = resetValue;
            changed = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f"))
            changed = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Z
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (ImGui::Button("Z", buttonSize))
        {
            values[2] = resetValue;
            changed = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f"))
            changed = true;
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return changed;
    }
} // namespace Editor