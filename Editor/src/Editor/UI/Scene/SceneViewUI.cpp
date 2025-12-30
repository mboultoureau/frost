#include "Editor/UI/Scene/SceneView.h"

#include "Frost/Asset/MeshConfig.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Disabled.h"
#include "Frost/Scene/Components/Prefab.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Debugging/ComponentUIRegistry.h"
#include "Frost/Debugging/DebugInterface/DebugUtils.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Transform.h"

#include "Editor/EditorLayer.h"
#include "Editor/Utils/FileDialogs.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <Frost/Scene/PrefabSerializer.h>

#include <algorithm>

namespace Editor
{
    using namespace Frost;
    using namespace Frost::Math;
    using namespace Frost::Component;

    void SceneView::Draw(float deltaTime)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        bool open = true;
        ImGui::Begin(_title.c_str(), &open);
        _isOpen = open;

        _isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        _isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        _DrawToolbar();

        if (ImGui::BeginPopup("SaveErrorPopup"))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Save Failed!");
            ImGui::Separator();
            ImGui::Text("A Prefab must contain exactly one root object.");
            ImGui::Button("Close");
            ImGui::EndPopup();
        }

        ImVec2 viewportMinRegion = ImGui::GetCursorScreenPos();
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        viewportPanelSize.x = std::max(viewportPanelSize.x, 1.0f);
        viewportPanelSize.y = std::max(viewportPanelSize.y, 1.0f);

        _ResizeViewportFramebuffer((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);

        if (_viewportTexture)
        {
            ImGui::Image((ImTextureID)_viewportTexture->GetRendererID(), viewportPanelSize);
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                std::filesystem::path assetPath = std::filesystem::path((const wchar_t*)payload->Data);

                if (assetPath.extension() == ".prefab")
                {
                    Vector3 spawnPos = _GetSpawnPositionFromMouse();
                    auto newEntity = _sceneContext->CreateGameObject(assetPath.stem().string());

                    newEntity.AddComponent<Transform>(spawnPos);
                    newEntity.AddComponent<Prefab>(assetPath);

                    if (_isPrefabView)
                    {
                        Frost::GameObject prefabRoot = _GetPrefabRoot();
                        if (prefabRoot)
                        {
                            newEntity.SetParent(prefabRoot);
                        }
                    }

                    _ClearSelection();
                    _AddToSelection(newEntity);
                }
                else if (std::find(MESH_FILE_EXTENSIONS.begin(),
                                   MESH_FILE_EXTENSIONS.end(),
                                   assetPath.extension().string()) != MESH_FILE_EXTENSIONS.end())
                {
                    _HandleMeshDrop(assetPath);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Gizmos
        if (!_selection.empty() && _currentGizmoOp != GizmoOperation::None)
        {
            bool hasTransform = false;
            for (auto& go : _selection)
            {
                if (go.HasComponent<Transform>())
                {
                    hasTransform = true;
                    break;
                }
            }

            if (hasTransform)
            {
                auto& cameraComponent = _editorCamera.GetComponent<Camera>();
                auto& cameraTransform = _editorCamera.GetComponent<Transform>();

                float aspectRatio = viewportPanelSize.x / viewportPanelSize.y;
                Matrix4x4 viewMatrix = GetViewMatrix(cameraTransform);
                Matrix4x4 projectionMatrix = GetProjectionMatrix(cameraComponent, aspectRatio);

                ImVec2 mousePos = ImGui::GetMousePos();
                float mx = mousePos.x - viewportMinRegion.x;
                float my = mousePos.y - viewportMinRegion.y;
                auto [rayOrigin, rayDir] = _GetCameraRay(mx, my, viewportPanelSize.x, viewportPanelSize.y);

                _gizmo->Update(_currentGizmoOp,
                               _selection,
                               viewMatrix,
                               projectionMatrix,
                               rayOrigin,
                               rayDir,
                               cameraTransform.position,
                               cameraTransform.GetForward(),
                               viewportMinRegion,
                               viewportPanelSize);
            }
        }

        if (_editorCamera && _editorCamera.HasComponent<Transform>())
        {
            auto& tc = _editorCamera.GetComponent<Transform>();
            _cameraController.OnUpdate(deltaTime, tc, _isHovered && !_gizmo->IsManipulating());
        }

        ImGui::End();
        ImGui::PopStyleVar();
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
                if (auto* meta = _sceneContext->GetRegistry().try_get<Meta>(entity);
                    meta && meta->name.find("__EDITOR__") == 0)
                    continue;
                if (view.get<Relationship>(entity).parent == entt::null)
                {
                    prefabRoot = entity;
                    rootCount++;
                }
            }
            if (rootCount == 0)
            {
                prefabRoot = (entt::entity)_sceneContext->CreateGameObject("Prefab Root").GetId();
            }
        }

        if (!_isReadOnly && ImGui::BeginPopupContextWindow("HierarchyContext", 1))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                auto newEntity = _sceneContext->CreateGameObject("Entity");
                if (_isPrefabView && prefabRoot != entt::null)
                {
                    newEntity.SetParent(Frost::GameObject(prefabRoot, _sceneContext));
                }

                _ClearSelection();
                _AddToSelection(newEntity);
            }
            ImGui::EndPopup();
        }

        std::vector<entt::entity> roots;
        _sceneContext->GetRegistry().view<Relationship>().each(
            [&](auto entityID, auto& relation)
            {
                if (relation.parent == entt::null)
                    roots.push_back(entityID);
            });

        std::sort(roots.begin(),
                  roots.end(),
                  [&](entt::entity a, entt::entity b)
                  {
                      auto* metaA = _sceneContext->GetRegistry().try_get<Meta>(a);
                      auto* metaB = _sceneContext->GetRegistry().try_get<Meta>(b);
                      return (metaA ? metaA->name : "") < (metaB ? metaB->name : "");
                  });

        for (auto entityID : roots)
        {
            _DrawEntityNode(entityID);
        }

        if (!_isReadOnly &&
            ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->Rect(), ImGui::GetID("HierarchyContent")))
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_NODE"))
            {
                entt::entity payloadEntity = *(entt::entity*)payload->Data;
                _ReparentEntity(payloadEntity,
                                _isPrefabView && prefabRoot != entt::null && payloadEntity != prefabRoot ? prefabRoot
                                                                                                         : entt::null);
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                std::filesystem::path assetPath = std::filesystem::path((const wchar_t*)payload->Data);
                if (assetPath.extension() == ".prefab")
                {
                    Frost::PrefabSerializer::Instantiate(_sceneContext, assetPath);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && !ImGui::GetIO().KeyCtrl)
        {
            _ClearSelection();
        }
    }

    void SceneView::_DrawEntityNode(entt::entity entityID)
    {
        auto& registry = _sceneContext->GetRegistry();
        if (!registry.valid(entityID))
            return;

        auto* meta = registry.try_get<Meta>(entityID);
        if (meta && meta->name.find("__EDITOR__") == 0)
            return;

        auto* relation = registry.try_get<Relationship>(entityID);
        bool isDisabled = registry.all_of<Disabled>(entityID);
        bool isPrefabRoot = _isPrefabView && relation && relation->parent == entt::null;
        bool isPrefabInstance = registry.all_of<Prefab>(entityID);

        std::string name = meta ? meta->name : "Entity " + std::to_string((uint32_t)entityID);

        Frost::GameObject currentGO(entityID, _sceneContext);
        bool isSelected = _IsSelected(currentGO);

        ImGuiTreeNodeFlags flags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;

        if (isPrefabInstance)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
            flags &= ~ImGuiTreeNodeFlags_OpenOnArrow;
        }
        else if (relation && relation->childrenCount == 0)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        if (isDisabled)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        if (isPrefabInstance)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entityID, flags, name.c_str());

        if (isPrefabInstance)
            ImGui::PopStyleColor();

        if (isDisabled)
            ImGui::PopStyleColor();

        if (ImGui::IsItemClicked())
        {
            bool ctrlPressed = ImGui::GetIO().KeyCtrl;

            if (ctrlPressed)
            {
                if (isSelected)
                    _RemoveFromSelection(currentGO);
                else
                    _AddToSelection(currentGO);
            }
            else
            {
                // Mode Standard
                if (!isSelected)
                {
                    _ClearSelection();
                    _AddToSelection(currentGO);
                }
            }
        }

        if (!_isReadOnly && !isPrefabRoot && ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("HIERARCHY_NODE", &entityID, sizeof(entt::entity));
            if (_selection.size() > 1 && isSelected)
                ImGui::Text("%d entities", (int)_selection.size());
            else
                ImGui::Text("%s", name.c_str());

            ImGui::EndDragDropSource();
        }

        if (!_isReadOnly && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_NODE"))
            {
                entt::entity payloadEntity = *(entt::entity*)payload->Data;

                if (payloadEntity != entityID)
                    _ReparentEntity(payloadEntity, entityID);
            }
            ImGui::EndDragDropTarget();
        }

        bool entityDeleted = false;
        if (!_isReadOnly && ImGui::BeginPopupContextItem())
        {
            if (!isSelected && !ImGui::GetIO().KeyCtrl)
            {
                _ClearSelection();
                _AddToSelection(currentGO);
            }

            if (ImGui::MenuItem("Create Child"))
            {
                auto child = _sceneContext->CreateGameObject("New Entity");
                child.SetParent(currentGO);
            }

            if (!isPrefabRoot && ImGui::MenuItem("Duplicate"))
            {
                _sceneContext->DuplicateGameObject(currentGO);
            }

            if (!isPrefabRoot && ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
        {
            if (relation && !isPrefabInstance)
            {
                auto sortedChildren = _GetSortedChildren(entityID);
                for (auto childID : sortedChildren)
                {
                    _DrawEntityNode(childID);
                }
            }
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            if (_IsSelected(currentGO))
                _RemoveFromSelection(currentGO);

            _sceneContext->DestroyGameObject(currentGO);
        }
    }

    void SceneView::OnRenderInspector()
    {
        if (_selection.empty())
        {
            ImGui::TextDisabled("Select an entity to view properties.");
            return;
        }

        if (_selection.size() > 1)
        {
            ImGui::Text("Multi-object editing not supported.");
            ImGui::TextDisabled("%d objects selected", (int)_selection.size());
            ImGui::Separator();
            return;
        }

        Frost::GameObject activeSelection = _selection[0];

        if (activeSelection.HasComponent<Prefab>())
        {
            auto& prefabComponent = activeSelection.GetComponent<Prefab>();
            std::string assetName = prefabComponent.assetPath.filename().string();

            ImGui::Text("Prefab Instance");
            ImGui::Separator();

            Frost::UIContext ctx{ !_isReadOnly, ImGui::GetIO().DeltaTime };

            Frost::ComponentUIRegistry::Draw<Meta>(_sceneContext, activeSelection, ctx);
            Frost::ComponentUIRegistry::Draw<Transform>(_sceneContext, activeSelection, ctx);

            if (Frost::DebugUtils::DrawComponentHeader("Prefab", nullptr))
            {
                if (ImGui::Button("Open Prefab Asset"))
                {
                    EditorLayer::Get().OpenPrefab(prefabComponent.assetPath);
                }

                if (ImGui::Button("Unpack Prefab"))
                {
                    auto transform = activeSelection.GetComponent<Transform>();
                    auto parent = activeSelection.GetParent();
                    std::string name = activeSelection.GetComponent<Meta>().name;
                    bool wasEnabled = !activeSelection.HasComponent<Disabled>();

                    _sceneContext->DestroyGameObject(activeSelection);
                    _ClearSelection();

                    auto newRoot = Frost::PrefabSerializer::Instantiate(_sceneContext, prefabComponent.assetPath);
                    if (newRoot)
                    {
                        newRoot.SetParent(parent);
                        newRoot.GetComponent<Transform>() = transform;
                        newRoot.GetComponent<Meta>().name = name;

                        if (wasEnabled)
                            newRoot.RemoveComponent<Disabled>();
                        else if (!newRoot.HasComponent<Disabled>())
                            newRoot.AddComponent<Disabled>();

                        _AddToSelection(newRoot);
                    }
                }

                ImGui::TreePop();
            }

            return;
        }

        Frost::UIContext ctx{ true, ImGui::GetIO().DeltaTime };
        Frost::ComponentUIRegistry::DrawAll(_sceneContext, (entt::entity)activeSelection, ctx);

        ImGui::Separator();
        if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                std::filesystem::path assetPath = std::filesystem::path((const wchar_t*)payload->Data);
                if (assetPath.extension() == ".prefab")
                {
                    if (activeSelection && !activeSelection.HasComponent<Frost::Component::Prefab>())
                    {
                        auto parent = activeSelection.GetParent();
                        auto transform = activeSelection.GetComponent<Transform>();
                        std::string originalName = activeSelection.GetComponent<Meta>().name;

                        _sceneContext->DestroyGameObject(activeSelection);
                        _ClearSelection();

                        auto newObj = Frost::PrefabSerializer::Instantiate(_sceneContext, assetPath);
                        if (newObj)
                        {
                            newObj.SetParent(parent);
                            newObj.GetComponent<Transform>() = transform;
                            newObj.GetComponent<Meta>().name = originalName;
                            _AddToSelection(newObj);
                        }
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            static char searchBuffer[64] = "";
            ImGui::InputTextWithHint("##SearchComponent", "Search...", searchBuffer, sizeof(searchBuffer));
            ImGui::Separator();

            for (const auto& serializer : SerializationSystem::GetAllSerializers())
            {
                if ((searchBuffer[0] != '\0' && serializer.Name.find(searchBuffer) == std::string::npos) ||
                    serializer.HasComponent(activeSelection) || serializer.Name == "Relationship" ||
                    serializer.Name == "Meta" || serializer.Name == "WorldTransform")
                {
                    continue;
                }
                if (ImGui::MenuItem(serializer.Name.c_str()))
                {
                    serializer.AddComponent(activeSelection);
                    ImGui::CloseCurrentPopup();
                    searchBuffer[0] = '\0';
                }
            }
            ImGui::EndPopup();
        }
    }

    void SceneView::_DrawToolbar()
    {
        _toolbar.Draw(
            _cameraController,
            _currentGizmoOp,
            _viewSettings,
            _isPrefabView,
            [this]() { this->_SavePrefab(); },
            [this]() { this->_SaveScene(); },
            [this]() { this->_LoadScene(); });

        if (_viewSettings.showEditorSkybox && !_editorCamera.HasComponent<Skybox>())
        {
            _editorCamera.AddComponent<Skybox>(
                SkyboxSourceCubemap{ "./resources/editor/skyboxes/Cubemap_Sky_04-512x512.png" });
        }
        else if (!_viewSettings.showEditorSkybox && _editorCamera.HasComponent<Skybox>())
        {
            _editorCamera.RemoveComponent<Skybox>();
        }

        _editorLight.SetActive(_viewSettings.showEditorLight);
        _editorEnvironment.SetActive(_viewSettings.showEditorEnvironment);
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

        const ImVec4 colors[] = { { 0.8f, 0.1f, 0.15f, 1.0f },
                                  { 0.2f, 0.7f, 0.2f, 1.0f },
                                  { 0.1f, 0.25f, 0.8f, 1.0f } };
        const char* axes[] = { "X", "Y", "Z" };

        for (int i = 0; i < 3; ++i)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
            if (ImGui::Button(axes[i], buttonSize))
            {
                values[i] = resetValue;
                changed = true;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushID(i);
            if (ImGui::DragFloat("##", &values[i], 0.1f, 0.0f, 0.0f, "%.2f"))
                changed = true;
            ImGui::PopID();
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine();
        }

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return changed;
    }

    std::vector<entt::entity> SceneView::_GetSortedChildren(entt::entity parentID)
    {
        auto& registry = _sceneContext->GetRegistry();
        std::vector<entt::entity> children;

        auto* relation = registry.try_get<Relationship>(parentID);
        if (!relation)
            return children;

        entt::entity currentChild = relation->firstChild;
        while (registry.valid(currentChild))
        {
            children.push_back(currentChild);
            auto* childRel = registry.try_get<Relationship>(currentChild);
            currentChild = childRel ? childRel->nextSibling : entt::null;
        }

        std::sort(children.begin(),
                  children.end(),
                  [&](entt::entity a, entt::entity b)
                  {
                      auto* metaA = registry.try_get<Meta>(a);
                      auto* metaB = registry.try_get<Meta>(b);

                      std::string nameA = metaA ? metaA->name : "Entity";
                      std::string nameB = metaB ? metaB->name : "Entity";

                      return std::lexicographical_compare(nameA.begin(),
                                                          nameA.end(),
                                                          nameB.begin(),
                                                          nameB.end(),
                                                          [](char c1, char c2)
                                                          { return std::tolower(c1) < std::tolower(c2); });
                  });

        return children;
    }
} // namespace Editor