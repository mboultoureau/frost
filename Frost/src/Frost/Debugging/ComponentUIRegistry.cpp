#include "Frost/Debugging/ComponentUIRegistry.h"

#include "Frost/Asset/MeshConfig.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Components/HUDImage.h"
#include "Frost/Scene/Components/UIButton.h"
#include "Frost/Scene/Components/Disabled.h"

#include "Frost/Debugging/DebugInterface/DebugUtils.h"
#include "Frost/Utils/Math/Angle.h"
#include <imgui.h>

namespace Frost
{
    using namespace Component;

    std::unordered_map<std::type_index, std::function<void(Scene*, entt::entity, const UIContext&)>>
        ComponentUIRegistry::_drawers;

    static entt::entity s_TransformLastEntity = entt::null;
    static Math::Vector3 s_TransformEulerCache = { 0.0f, 0.0f, 0.0f };

    void ComponentUIRegistry::DrawAll(Scene* scene, entt::entity e, const UIContext& ctx)
    {
        if (!scene || !scene->GetRegistry().valid(e))
        {
            return;
        }

        for (const auto& [typeIndex, drawer] : _drawers)
        {
            drawer(scene, e, ctx);
        }
    }

    void ComponentUIRegistry::InitEngineComponents()
    {
        // Meta and Disabled
        Register<Meta>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& meta = scene->GetRegistry().get<Meta>(e);

                if (DebugUtils::DrawComponentHeader("Meta", nullptr))
                {
                    bool isEnabled = !scene->GetRegistry().all_of<Disabled>(e);

                    ImGui::AlignTextToFramePadding();
                    if (ImGui::Checkbox("##Active", &isEnabled))
                    {
                        if (isEnabled)
                        {
                            scene->GetRegistry().remove<Disabled>(e);
                        }
                        else
                        {
                            scene->GetRegistry().emplace<Disabled>(e);
                        }
                    }
                    ImGui::SameLine();

                    if (ctx.isEditor)
                    {
                        char nameBuffer[256];
                        strncpy_s(nameBuffer, meta.name.c_str(), sizeof(nameBuffer) - 1);
                        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
                        {
                            meta.name = std::string(nameBuffer);
                        }
                    }
                    else
                    {
                        ImGui::Text("ID: %llu", (uint64_t)e);
                        ImGui::Text("Name: %s", meta.name.c_str());
                    }

                    ImGui::TreePop();
                }
            });

        // Transform
        Register<Transform>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& transform = scene->GetRegistry().get<Transform>(e);

                if (DebugUtils::DrawComponentHeader("Transform", nullptr))
                {
                    // Position
                    DebugUtils::DrawVec3Control("Position", transform.position);

                    // Rotation
                    if (e != s_TransformLastEntity)
                    {
                        Math::EulerAngles eulerRad = transform.GetEulerAngles();
                        s_TransformEulerCache = { Math::Angle<Math::Degree>(eulerRad.Pitch).value(),
                                                  Math::Angle<Math::Degree>(eulerRad.Yaw).value(),
                                                  Math::Angle<Math::Degree>(eulerRad.Roll).value() };
                        s_TransformLastEntity = e;
                    }

                    if (DebugUtils::DrawVec3Control("Rotation", s_TransformEulerCache))
                    {
                        Math::EulerAngles newAnglesRad = {
                            .Roll = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(s_TransformEulerCache.z)),
                            .Yaw = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(s_TransformEulerCache.y)),
                            .Pitch = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(s_TransformEulerCache.x))
                        };
                        transform.SetRotation(newAnglesRad);
                    }

                    // Scale
                    DebugUtils::DrawVec3Control("Scale", transform.scale, 1.0f);

                    ImGui::TreePop();
                }
            });

        // Camera
        Register<Camera>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& camera = scene->GetRegistry().get<Camera>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("Camera", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<Camera>(e);
                        ImGui::TreePop();
                        return;
                    }

                    const char* projectionTypes[] = { "Perspective", "Orthographic" };
                    int currentType = (int)camera.projectionType;
                    if (ImGui::Combo("Projection Type", &currentType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
                    {
                        camera.projectionType = (Camera::ProjectionType)currentType;
                    }

                    if (camera.projectionType == Camera::ProjectionType::Perspective)
                    {
                        float currentFOVDegrees = Math::Angle<Math::Degree>(camera.perspectiveFOV).value();
                        if (ImGui::DragFloat("FOV", &currentFOVDegrees, 0.5f, 30.0f, 120.0f))
                            camera.perspectiveFOV = Math::Angle<Math::Radian>{
                                Math::angle_cast<Math::Radian, Math::Degree>(currentFOVDegrees)
                            };
                    }
                    else
                    {
                        ImGui::DragFloat("Size", &camera.orthographicSize, 0.1f, 1.0f, 1000.0f);
                    }

                    ImGui::DragFloat("Near Clip", &camera.nearClip, 0.01f);
                    ImGui::DragFloat("Far Clip", &camera.farClip, 1.0f);

                    ImGui::Checkbox("Clear On Render", &camera.clearOnRender);
                    ImGui::ColorEdit4("Background Color", camera.backgroundColor.values);
                    ImGui::DragInt("Priority", &camera.priority);

                    if (!ctx.isEditor)
                    {
                        ImGui::TextDisabled("Viewport: %.2f, %.2f", camera.viewport.width, camera.viewport.height);
                    }

                    ImGui::TreePop();
                }
            });

        // Static Mesh
        Register<StaticMesh>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& mesh = scene->GetRegistry().get<StaticMesh>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("Static Mesh", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<StaticMesh>(e);
                        ImGui::TreePop();
                        return;
                    }

                    const char* meshTypeNames[] = { "File", "Cube", "Sphere", "Plane", "Cylinder", "HeightMap" };
                    int currentType = (int)mesh.GetMeshConfig().index();

                    if (ImGui::Combo("Source Type", &currentType, meshTypeNames, IM_ARRAYSIZE(meshTypeNames)))
                    {
                        switch ((MeshType)currentType)
                        {
                            case MeshType::File:
                                mesh.SetMeshConfig(MeshSourceFile{});
                                break;
                            case MeshType::Cube:
                                mesh.SetMeshConfig(MeshSourceCube{});
                                break;
                            case MeshType::Sphere:
                                mesh.SetMeshConfig(MeshSourceSphere{});
                                break;
                            case MeshType::Plane:
                                mesh.SetMeshConfig(MeshSourcePlane{});
                                break;
                            case MeshType::Cylinder:
                                mesh.SetMeshConfig(MeshSourceCylinder{});
                                break;
                            case MeshType::HeightMap:
                                mesh.SetMeshConfig(MeshSourceHeightMap{});
                                break;
                        }
                    }

                    ImGui::Separator();

                    bool configChanged = false;

                    if (auto* p = std::get_if<MeshSourceFile>(&mesh.GetMeshConfig()))
                    {
                        char buffer[256];
                        memset(buffer, 0, sizeof(buffer));
                        strncpy_s(buffer, p->filepath.string().c_str(), sizeof(buffer));

                        if (ImGui::InputText("File Path", buffer, sizeof(buffer)))
                        {
                            p->filepath = std::string(buffer);
                            configChanged = true;
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                const wchar_t* path = (const wchar_t*)payload->Data;
                                std::filesystem::path filePath(path);

                                std::string ext = filePath.extension().string();
                                if (std::find(MESH_FILE_EXTENSIONS.begin(), MESH_FILE_EXTENSIONS.end(), ext) !=
                                    MESH_FILE_EXTENSIONS.end())
                                {
                                    p->filepath = filePath.string();
                                    configChanged = true;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }

                        if (ImGui::Button("Reload Model"))
                        {
                            configChanged = true;
                        }

                        if (!mesh.GetModel())
                        {
                            ImGui::TextColored({ 1, 0, 0, 1 }, "Error: Model not loaded.");
                        }
                    }
                    else if (auto* p = std::get_if<MeshSourceCube>(&mesh.GetMeshConfig()))
                    {
                        if (ImGui::DragFloat("Size", &p->size, 0.1f, 0.01f, 1000.0f))
                            configChanged = true;

                        float maxBevel = p->size * 0.5f;
                        if (ImGui::DragFloat("Bevel Radius", &p->bevelRadius, 0.05f, 0.0f, maxBevel))
                        {
                            // Clamp pour sécurité
                            if (p->bevelRadius > maxBevel)
                                p->bevelRadius = maxBevel;
                            if (p->bevelRadius < 0.0f)
                                p->bevelRadius = 0.0f;
                            configChanged = true;
                        }

                        ImGui::BeginDisabled(p->bevelRadius > 0.001f);

                        int segs[3] = { (int)p->segments.x, (int)p->segments.y, (int)p->segments.z };
                        if (ImGui::DragInt3("Segments", segs, 1, 1, 64))
                        {
                            p->segments = { (float)segs[0], (float)segs[1], (float)segs[2] };
                            configChanged = true;
                        }

                        if (p->bevelRadius > 0.001f && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip("Subdivision is controlled by Bevel logic when Bevel Radius > 0");
                        }

                        ImGui::EndDisabled();
                    }
                    else if (auto* p = std::get_if<MeshSourceSphere>(&mesh.GetMeshConfig()))
                    {
                        if (ImGui::DragFloat("Radius", &p->radius, 0.1f, 0.01f, 1000.0f))
                            configChanged = true;

                        int rings = (int)p->rings;
                        int slices = (int)p->slices;

                        if (ImGui::DragInt("Rings", &rings, 1, 3, 128))
                        {
                            p->rings = (uint32_t)rings;
                            configChanged = true;
                        }
                        if (ImGui::DragInt("Slices", &slices, 1, 3, 128))
                        {
                            p->slices = (uint32_t)slices;
                            configChanged = true;
                        }
                    }
                    else if (auto* p = std::get_if<MeshSourcePlane>(&mesh.GetMeshConfig()))
                    {
                        if (ImGui::DragFloat("Width", &p->width, 0.1f, 0.01f, 10000.0f))
                        {
                            configChanged = true;
                        }
                        if (ImGui::DragFloat("Depth", &p->depth, 0.1f, 0.01f, 10000.0f))
                        {
                            configChanged = true;
                        }
                    }
                    else if (auto* p = std::get_if<MeshSourceCylinder>(&mesh.GetMeshConfig()))
                    {
                        if (ImGui::DragFloat("Bottom Radius", &p->bottomRadius, 0.1f, 0.0f, 1000.0f))
                            configChanged = true;

                        if (ImGui::DragFloat("Top Radius", &p->topRadius, 0.1f, 0.0f, 1000.0f))
                            configChanged = true;

                        if (ImGui::DragFloat("Height", &p->height, 0.1f, 0.01f, 1000.0f))
                            configChanged = true;

                        int slices = (int)p->slices;
                        int stacks = (int)p->stacks;

                        if (ImGui::DragInt("Slices", &slices, 1, 3, 128))
                        {
                            p->slices = (uint32_t)slices;
                            configChanged = true;
                        }
                        if (ImGui::DragInt("Stacks", &stacks, 1, 1, 128))
                        {
                            p->stacks = (uint32_t)stacks;
                            configChanged = true;
                        }
                    }
                    else if (auto* p = std::get_if<MeshSourceHeightMap>(&mesh.GetMeshConfig()))
                    {
                        ImGui::Text("Heightmap Configuration");
                        ImGui::Spacing();
                        ImGui::Text("Source Texture");

                        float availableWidth = ImGui::GetContentRegionAvail().x;
                        ImVec2 buttonSize = { availableWidth, 80.0f };

                        std::string buttonLabel = "Drop Texture Here";
                        if (!p->texturePath.empty())
                        {
                            std::filesystem::path path(p->texturePath);
                            buttonLabel = path.filename().string();
                        }

                        ImGui::Button(buttonLabel.c_str(), buttonSize);

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                const wchar_t* path = (const wchar_t*)payload->Data;
                                std::filesystem::path filePath(path);

                                std::string ext = filePath.extension().string();
                                if (std::find(HEIGHTMAP_EXTENSIONS.begin(), HEIGHTMAP_EXTENSIONS.end(), ext) !=
                                    HEIGHTMAP_EXTENSIONS.end())
                                {
                                    p->texturePath = filePath.string();
                                    configChanged = true;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }

                        if (!p->texturePath.empty())
                        {
                            if (ImGui::Button("Clear Texture"))
                            {
                                p->texturePath = "";
                                configChanged = true;
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Reload Gen"))
                            {
                                configChanged = true;
                            }
                        }

                        ImGui::Separator();

                        ImGui::Text("Dimensions");
                        if (ImGui::DragFloat("Width", &p->width, 0.1f, 0.1f, 10000.0f))
                            configChanged = true;
                        if (ImGui::DragFloat("Depth", &p->depth, 0.1f, 0.1f, 10000.0f))
                            configChanged = true;

                        ImGui::Text("Altitude");
                        if (ImGui::DragFloat("Min Height", &p->minHeight, 0.1f))
                            configChanged = true;
                        if (ImGui::DragFloat("Max Height", &p->maxHeight, 0.1f))
                            configChanged = true;

                        ImGui::Text("Resolution");
                        int seg[2] = { (int)p->segmentsWidth, (int)p->segmentsDepth };
                        if (ImGui::DragInt2("Segments (X/Z)", seg, 1, 2, 512))
                        {
                            p->segmentsWidth = (uint32_t)seg[0];
                            p->segmentsDepth = (uint32_t)seg[1];
                            configChanged = true;
                        }

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Higher values = more vertices (slower)");
                    }

                    if (configChanged)
                    {
                        mesh.Reload();
                    }

                    ImGui::TreePop();
                }
            });

        Register<Light>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& light = scene->GetRegistry().get<Light>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("Light", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<Light>(e);
                        ImGui::TreePop();
                        return;
                    }

                    const char* lightTypes[] = { "Directional (Sun)", "Point", "Spot" };
                    int currentType = (int)light.GetType();

                    if (ImGui::Combo("Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes)))
                    {
                        light.SetType((LightType)currentType);
                    }

                    ImGui::Separator();
                    ImGui::Text("Emission Settings");

                    static bool useTemperature = false;
                    static float temperatureK = 6500.0f;

                    ImGui::Checkbox("Use Color Temp (K)", &useTemperature);

                    if (useTemperature)
                    {
                        if (ImGui::SliderFloat("Kelvin", &temperatureK, 1000.0f, 12000.0f, "%.0f K"))
                        {
                            light.color = Math::KelvinToRGB(temperatureK);
                        }
                        ImGui::SameLine();
                        ImGui::ColorButton("##preview", { light.color.r, light.color.g, light.color.b, 1.0f });
                    }
                    else
                    {
                        ImGui::ColorEdit3("Color", light.color.values);
                    }

                    ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f, 10000.0f, "%.2f cd");

                    ImGui::Separator();

                    if (auto* dir = std::get_if<LightDirectional>(&light.config))
                    {
                        ImGui::Text("Directional Parameters");
                        ImGui::Checkbox("Cast Shadows", &dir->castShadows);
                        if (dir->castShadows)
                        {
                            ImGui::DragFloat("Shadow Bias", &dir->shadowBias, 0.001f, 0.0f, 1.0f, "%.4f");
                        }

                        ImGui::TextDisabled("Rotation is controlled by the Transform component.");
                    }
                    else if (auto* point = std::get_if<LightPoint>(&light.config))
                    {
                        ImGui::Text("Point Parameters");
                        ImGui::DragFloat("Range", &point->radius, 0.1f, 0.1f, 1000.0f);

                        ImGui::DragFloat("Falloff", &point->falloff, 0.05f, 0.1f, 10.0f);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("1.0 = Linear, 2.0 = Realistic (Inverse Square)");
                        }
                    }
                    else if (auto* spot = std::get_if<LightSpot>(&light.config))
                    {
                        ImGui::Text("Spot Parameters");
                        ImGui::DragFloat("Range", &spot->range, 0.1f, 0.1f, 1000.0f);

                        float innerDeg = Math::Angle<Math::Degree>(spot->innerConeAngle).value();
                        float outerDeg = Math::Angle<Math::Degree>(spot->outerConeAngle).value();

                        bool anglesChanged = false;

                        if (ImGui::DragFloat("Inner Angle", &innerDeg, 0.5f, 1.0f, 179.0f, "%.1f deg"))
                        {
                            if (innerDeg >= outerDeg)
                            {
                                outerDeg = innerDeg + 1.0f;
                            }
                            anglesChanged = true;
                        }

                        if (ImGui::DragFloat("Outer Angle", &outerDeg, 0.5f, 1.0f, 179.0f, "%.1f deg"))
                        {
                            if (outerDeg <= innerDeg)
                            {
                                outerDeg = innerDeg + 1.0f;
                            }
                            anglesChanged = true;
                        }

                        if (anglesChanged)
                        {
                            spot->innerConeAngle = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(innerDeg));
                            spot->outerConeAngle = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(outerDeg));
                        }
                    }

                    ImGui::TreePop();
                }
            });
    }
} // namespace Frost