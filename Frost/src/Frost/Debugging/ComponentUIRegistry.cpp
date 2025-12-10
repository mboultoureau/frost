#include "Frost/Debugging/ComponentUIRegistry.h"

#include "Frost/Asset/AssetManager.h"
#include "Frost/Asset/MeshConfig.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Components/Prefab.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Scene/Components/UIElement.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Scene/Components/Disabled.h"
#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scripting/ScriptingEngine.h"
#include "Frost/Physics/Physics.h"

#include "Frost/Debugging/DebugInterface/DebugUtils.h"
#include "Frost/Utils/Math/Angle.h"
#include <imgui.h>

namespace Frost
{
    using namespace Component;

    static entt::entity s_TransformLastEntity = entt::null;
    static Math::Vector3 s_TransformEulerCache = { 0.0f, 0.0f, 0.0f };

    const std::vector<std::string> IMAGE_FILE_EXTENSIONS = { ".png", ".jpg", ".jpeg", ".tga", ".bmp", ".hdr", ".dds" };
    const std::vector<std::string> FONT_FILE_EXTENSIONS = { ".ttf", ".otf" };

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
        FT_ENGINE_INFO("Initializing Component UI Registry for Engine Components...");

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
                    bool modified = false;

                    // Position
                    modified |= DebugUtils::DrawVec3Control("Position", transform.position);

                    // Rotation
                    if (e != s_TransformLastEntity)
                    {
                        Math::EulerAngles eulerRad = transform.GetEulerAngles();
                        s_TransformEulerCache = { Math::Angle<Math::Degree>(eulerRad.Pitch).value(),
                                                  Math::Angle<Math::Degree>(eulerRad.Yaw).value(),
                                                  Math::Angle<Math::Degree>(eulerRad.Roll).value() };
                        s_TransformLastEntity = e;
                    }

                    if (modified |= DebugUtils::DrawVec3Control("Rotation", s_TransformEulerCache))
                    {
                        Math::EulerAngles newAnglesRad = {
                            .Roll = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(s_TransformEulerCache.z)),
                            .Yaw = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(s_TransformEulerCache.y)),
                            .Pitch = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(s_TransformEulerCache.x))
                        };
                        transform.SetRotation(newAnglesRad);
                    }

                    // Scale
                    modified |= DebugUtils::DrawVec3Control("Scale", transform.scale, 1.0f);

                    if (modified)
                    {
                        if (auto* physicSystem = scene->GetSystem<PhysicSystem>())
                        {
                            physicSystem->NotifyRigidBodyUpdate(*scene, GameObject{ e });
                            FT_ENGINE_TRACE("RigidBody component on entity {} updated in UI.", (uint64_t)e);
                        }
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

        // Light
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

        // RigidBody
        Register<RigidBody>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& rb = scene->GetRegistry().get<RigidBody>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("Rigid Body", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<RigidBody>(e);
                        ImGui::TreePop();
                        return;
                    }

                    bool modified = false;

                    const char* motionTypes[] = { "Static", "Kinematic", "Dynamic" };
                    int currentMotionType = static_cast<int>(rb.motionType);
                    if (ImGui::Combo("Motion Type", &currentMotionType, motionTypes, IM_ARRAYSIZE(motionTypes)))
                    {
                        rb.motionType = static_cast<RigidBody::MotionType>(currentMotionType);
                        modified = true;
                    }

                    ImGui::Separator();
                    ImGui::Text("Collision Settings");

                    // Object Layer
                    const auto& layers = Physics::GetLayerNames();

                    if (!layers.empty())
                    {
                        int currentLayerIndex = -1;
                        std::string previewValue = "Unknown (" + std::to_string(rb.objectLayer) + ")";

                        std::vector<const char*> layerNamesCStr;
                        layerNamesCStr.reserve(layers.size());

                        for (int i = 0; i < layers.size(); ++i)
                        {
                            layerNamesCStr.push_back(layers[i].name.c_str());
                            if (layers[i].layerId == rb.objectLayer)
                            {
                                currentLayerIndex = i;
                                previewValue = layers[i].name;
                            }
                        }

                        if (ImGui::Combo(
                                "Layer", &currentLayerIndex, layerNamesCStr.data(), (int)layerNamesCStr.size()))
                        {
                            if (currentLayerIndex >= 0 && currentLayerIndex < layers.size())
                            {
                                rb.objectLayer = layers[currentLayerIndex].layerId;
                                modified = true;
                            }
                        }
                    }
                    else
                    {
                        // Raw input if no layers are defined
                        int rawLayer = (int)rb.objectLayer;
                        if (ImGui::InputInt("Layer ID", &rawLayer))
                        {
                            rb.objectLayer = (JPH::ObjectLayer)rawLayer;
                            modified = true;
                        }
                    }

                    ImGui::Separator();
                    ImGui::Text("Collision Shape");

                    const char* shapeTypes[] = { "Box", "Sphere", "Capsule", "Cylinder", "Mesh" };
                    int currentShapeType = static_cast<int>(rb.shape.index());
                    if (ImGui::Combo("Shape", &currentShapeType, shapeTypes, IM_ARRAYSIZE(shapeTypes)))
                    {
                        switch (static_cast<CollisionShapeType>(currentShapeType))
                        {
                            case CollisionShapeType::Box:
                                rb.shape = ShapeBox{};
                                break;
                            case CollisionShapeType::Sphere:
                                rb.shape = ShapeSphere{};
                                break;
                            case CollisionShapeType::Capsule:
                                rb.shape = ShapeCapsule{};
                                break;
                            case CollisionShapeType::Cylinder:
                                rb.shape = ShapeCylinder{};
                                break;
                            case CollisionShapeType::Mesh:
                                rb.shape = ShapeMesh{};
                                break;
                        }

                        modified = true;
                    }

                    modified |= std::visit(
                        [](auto&& arg)
                        {
                            bool shapeModified = false;
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, ShapeBox>)
                            {
                                shapeModified |= DebugUtils::DrawVec3Control("Half Extent", arg.halfExtent, 0.0f, 0.1f);
                                shapeModified |=
                                    ImGui::DragFloat("Convex Radius", &arg.convexRadius, 0.005f, 0.0f, 1.0f);
                            }
                            else if constexpr (std::is_same_v<T, ShapeSphere>)
                            {
                                shapeModified |= ImGui::DragFloat("Radius", &arg.radius, 0.05f, 0.01f, 1000.0f);
                            }
                            else if constexpr (std::is_same_v<T, ShapeCapsule>)
                            {
                                shapeModified |= ImGui::DragFloat("Half Height", &arg.halfHeight, 0.05f, 0.0f, 1000.0f);
                                shapeModified |= ImGui::DragFloat("Radius", &arg.radius, 0.05f, 0.01f, 1000.0f);
                            }
                            else if constexpr (std::is_same_v<T, ShapeCylinder>)
                            {
                                shapeModified |= ImGui::DragFloat("Half Height", &arg.halfHeight, 0.05f, 0.0f, 1000.0f);
                                shapeModified |= ImGui::DragFloat("Radius", &arg.radius, 0.05f, 0.01f, 1000.0f);
                                shapeModified |=
                                    ImGui::DragFloat("Convex Radius", &arg.convexRadius, 0.005f, 0.0f, 1.0f);
                            }
                            else if constexpr (std::is_same_v<T, ShapeMesh>)
                            {
                                char buffer[256];
                                memset(buffer, 0, sizeof(buffer));
                                strncpy_s(buffer, sizeof(buffer), arg.path.c_str(), _TRUNCATE);

                                if (ImGui::InputText("Mesh Path", buffer, sizeof(buffer)))
                                {
                                    arg.path = std::string(buffer);
                                    shapeModified = true;
                                }

                                if (ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload* payload =
                                            ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                                    {
                                        const wchar_t* path = (const wchar_t*)payload->Data;
                                        std::filesystem::path filePath(path);

                                        std::string ext = filePath.extension().string();
                                        if (std::find(MESH_FILE_EXTENSIONS.begin(), MESH_FILE_EXTENSIONS.end(), ext) !=
                                            MESH_FILE_EXTENSIONS.end())
                                        {
                                            arg.path = filePath.string();
                                            shapeModified = true;
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                            }

                            return shapeModified;
                        },
                        rb.shape);

                    ImGui::Separator();
                    ImGui::Text("Material Properties");
                    modified |= ImGui::DragFloat("Friction", &rb.friction, 0.01f, 0.0f, 10.0f);
                    modified |= ImGui::DragFloat("Restitution", &rb.restitution, 0.01f, 0.0f, 1.0f);

                    if (rb.motionType == RigidBody::MotionType::Dynamic)
                    {
                        ImGui::Separator();
                        ImGui::Text("Mass Properties");

                        modified |= ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.01f, 10000.0f);
                        ImGui::SameLine();
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Only used if 'Override Mass' is not 'Calculate All'");
                    }

                    ImGui::Separator();
                    ImGui::Text("Simulation Settings");
                    modified |= ImGui::DragFloat("Linear Damping", &rb.linearDamping, 0.01f, 0.0f, 100.0f);
                    modified |= ImGui::DragFloat("Angular Damping", &rb.angularDamping, 0.01f, 0.0f, 100.0f);
                    modified |= ImGui::DragFloat("Gravity Factor", &rb.gravityFactor, 0.05f, -10.0f, 10.0f);

                    modified |= ImGui::Checkbox("Is Sensor (Trigger)", &rb.isSensor);
                    modified |= ImGui::Checkbox("Allow Sleeping", &rb.allowSleeping);

                    ImGui::Separator();
                    ImGui::Text("Axis Locks (Degrees of Freedom)");

                    ImGui::Columns(2, "dof_columns", false);
                    ImGui::Text("Translation");
                    ImGui::NextColumn();
                    ImGui::Text("Rotation");
                    ImGui::NextColumn();
                    ImGui::Separator();

                    modified |= ImGui::Checkbox("Lock X##Trans", &rb.lockPositionX);
                    ImGui::NextColumn();
                    modified |= ImGui::Checkbox("Lock X##Rot", &rb.lockRotationX);
                    ImGui::NextColumn();

                    modified |= ImGui::Checkbox("Lock Y##Trans", &rb.lockPositionY);
                    ImGui::NextColumn();
                    modified |= ImGui::Checkbox("Lock Y##Rot", &rb.lockRotationY);
                    ImGui::NextColumn();

                    modified |= ImGui::Checkbox("Lock Z##Trans", &rb.lockPositionZ);
                    ImGui::NextColumn();
                    modified |= ImGui::Checkbox("Lock Z##Rot", &rb.lockRotationZ);
                    ImGui::NextColumn();

                    ImGui::Columns(1);

                    if (modified)
                    {
                        if (auto* physicSystem = scene->GetSystem<PhysicSystem>())
                        {
                            physicSystem->NotifyRigidBodyUpdate(*scene, GameObject{ e });
                            FT_ENGINE_TRACE("RigidBody component on entity {} updated in UI.", (uint64_t)e);
                        }
                    }

                    ImGui::TreePop();
                }
            });

        // Scriptable
        Register<Scriptable>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                if (!scene->GetRegistry().all_of<Scriptable>(e))
                    return;

                auto& scriptable = scene->GetRegistry().get<Scriptable>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("Scriptable", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<Scriptable>(e);
                        ImGui::TreePop();
                        return;
                    }

                    int scriptToRemove = -1;
                    for (int i = 0; i < scriptable.scriptNames.size(); ++i)
                    {
                        ImGui::PushID(i);

                        if (ImGui::Button("[X]"))
                        {
                            scriptToRemove = i;
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled(scriptable.scriptNames[i].c_str());

                        ImGui::PopID();
                    }

                    if (scriptToRemove != -1)
                    {
                        scriptable.scriptNames.erase(scriptable.scriptNames.begin() + scriptToRemove);
                    }

                    ImGui::Separator();

                    auto availableScripts = Scripting::ScriptingEngine::GetAvailableScripts();
                    std::vector<std::string> scriptsToAdd;
                    for (const auto& available : availableScripts)
                    {
                        if (std::find(scriptable.scriptNames.begin(), scriptable.scriptNames.end(), available) ==
                            scriptable.scriptNames.end())
                        {
                            scriptsToAdd.push_back(available);
                        }
                    }

                    ImGui::PushID((void*)&scriptable);
                    static int currentItem = 0;

                    ImGui::BeginDisabled(scriptsToAdd.empty());
                    {
                        auto items_getter = [](void* data, int idx, const char** out_text)
                        {
                            auto* items = static_cast<std::vector<std::string>*>(data);
                            *out_text = (*items)[idx].c_str();
                            return true;
                        };
                        ImGui::Combo("##ScriptToAdd", &currentItem, items_getter, &scriptsToAdd, scriptsToAdd.size());

                        ImGui::SameLine();

                        if (ImGui::Button("Add Script"))
                        {
                            if (currentItem >= 0 && currentItem < scriptsToAdd.size())
                            {
                                scriptable.scriptNames.push_back(scriptsToAdd[currentItem]);
                                currentItem = 0;
                            }
                        }
                    }
                    ImGui::EndDisabled();

                    if (scriptsToAdd.empty())
                    {
                        ImGui::TextDisabled("No more scripts to add.");
                    }
                    ImGui::PopID();

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetItemRectSize().y -
                                         ImGui::GetStyle().ItemSpacing.y * 2);
                    ImGui::InvisibleButton("##script_drop_target", ImGui::GetItemRectSize());

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        {
                            const wchar_t* pathW = (const wchar_t*)payload->Data;
                            std::filesystem::path filePath(pathW);

                            std::string scriptName = filePath.stem().string();

                            if (std::find(availableScripts.begin(), availableScripts.end(), scriptName) !=
                                    availableScripts.end() &&
                                std::find(scriptable.scriptNames.begin(), scriptable.scriptNames.end(), scriptName) ==
                                    scriptable.scriptNames.end())
                            {
                                scriptable.scriptNames.push_back(scriptName);
                                FT_ENGINE_INFO("Added script '{}' via Drag and Drop.", scriptName);
                            }
                            else
                            {
                                FT_ENGINE_WARN("Could not add script '{}' via Drag and Drop. It might not exist or is "
                                               "already attached.",
                                               scriptName);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::TreePop();
                }
            });

        // Prefab
        Register<Prefab>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                if (!scene->GetRegistry().all_of<Prefab>(e))
                    return;

                auto& prefab = scene->GetRegistry().get<Prefab>(e);
                bool unpackRequested = false;

                if (DebugUtils::DrawComponentHeader("Prefab", &unpackRequested))
                {
                    if (unpackRequested)
                    {
                        FT_ENGINE_INFO("Unpacking Prefab instance for entity {}. It is now unlinked from its asset.",
                                       (uint32_t)e);
                        scene->GetRegistry().remove<Prefab>(e);
                        ImGui::TreePop();
                        return;
                    }

                    bool pathChanged = false;
                    std::filesystem::path newPath;

                    char pathBuffer[512];
                    memset(pathBuffer, 0, sizeof(pathBuffer));
                    strncpy_s(pathBuffer, sizeof(pathBuffer), prefab.assetPath.string().c_str(), _TRUNCATE);

                    ImGui::Text("Asset");
                    ImGui::SameLine();
                    ImGui::InputText("##PrefabPath", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_ReadOnly);

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        {
                            const wchar_t* pathW = (const wchar_t*)payload->Data;
                            std::filesystem::path droppedPath(pathW);

                            if (droppedPath.extension() == ".prefab")
                            {
                                if (prefab.assetPath != droppedPath)
                                {
                                    newPath = droppedPath;
                                    pathChanged = true;
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (pathChanged)
                    {
                        prefab.assetPath = newPath;
                        GameObject currentInstance(e, scene);
                        currentInstance.DestroyAllChildren();

                        if (!prefab.assetPath.empty() && std::filesystem::exists(prefab.assetPath))
                        {
                            GameObject tempRoot = PrefabSerializer::Instantiate(scene, prefab.assetPath);
                            if (tempRoot)
                            {
                                auto childrenToMove = tempRoot.GetChildren();
                                for (auto& child : childrenToMove)
                                {
                                    child.SetParent(currentInstance);
                                }

                                scene->DestroyGameObject(tempRoot);
                            }
                        }
                    }

                    ImGui::TreePop();
                }
            });

        // Skybox
        Register<Skybox>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                if (!scene->GetRegistry().all_of<Skybox>(e))
                    return;

                auto& skybox = scene->GetRegistry().get<Skybox>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("Skybox", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<Skybox>(e);
                        ImGui::TreePop();
                        return;
                    }

                    const char* skyboxTypes[] = { "Cubemap", "6 Files" };
                    int currentType = (int)skybox.GetType();

                    if (ImGui::Combo("Source Type", &currentType, skyboxTypes, IM_ARRAYSIZE(skyboxTypes)))
                    {
                        skybox.SetType(static_cast<SkyboxType>(currentType));
                    }

                    ImGui::DragFloat("Intensity", &skybox.intensity, 0.05f, 0.0f, 100.0f);

                    ImGui::Separator();

                    auto DrawPathInput = [&](const char* label, std::filesystem::path& path) -> bool
                    {
                        bool changed = false;
                        char buffer[256];
                        memset(buffer, 0, sizeof(buffer));
                        strncpy_s(buffer, path.string().c_str(), sizeof(buffer));

                        ImGui::InputText(label, buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                const wchar_t* payload_path = (const wchar_t*)payload->Data;
                                std::filesystem::path filePath(payload_path);
                                std::string ext = filePath.extension().string();

                                if (std::find(IMAGE_FILE_EXTENSIONS.begin(), IMAGE_FILE_EXTENSIONS.end(), ext) !=
                                    IMAGE_FILE_EXTENSIONS.end())
                                {
                                    path = filePath;
                                    changed = true;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                        return changed;
                    };

                    if (auto* p = std::get_if<SkyboxSourceCubemap>(&skybox.config))
                    {
                        DrawPathInput("Cubemap Path", p->filepath);
                    }
                    else if (auto* p = std::get_if<SkyboxSource6Files>(&skybox.config))
                    {
                        const char* faceLabels[] = { "Right (+X)",  "Left (-X)",  "Top (+Y)",
                                                     "Bottom (-Y)", "Front (+Z)", "Back (-Z)" };
                        for (int i = 0; i < 6; ++i)
                        {
                            DrawPathInput(faceLabels[i], p->faceFilepaths[i]);
                        }
                    }

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
                        float fovDegrees = Math::Angle<Math::Degree>(camera.perspectiveFOV).value();
                        if (ImGui::DragFloat("FOV", &fovDegrees, 0.5f, 30.0f, 120.0f))
                            camera.perspectiveFOV = Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(fovDegrees));
                    }
                    else
                    {
                        ImGui::DragFloat("Size", &camera.orthographicSize, 0.1f, 1.0f, 1000.0f);
                    }

                    ImGui::DragFloat("Near Clip", &camera.nearClip, 0.01f);
                    ImGui::DragFloat("Far Clip", &camera.farClip, 1.0f);
                    ImGui::DragInt("Priority", &camera.priority);

                    ImGui::Separator();

                    ImGui::Checkbox("Clear On Render", &camera.clearOnRender);
                    ImGui::ColorEdit4("Background Color", camera.backgroundColor.values);

                    ImGui::Separator();

                    bool isRenderTarget = camera.renderTargetConfig.has_value();
                    if (ImGui::Checkbox("Render To Texture", &isRenderTarget))
                    {
                        if (isRenderTarget)
                        {
                            camera.renderTargetConfig.emplace();
                        }
                        else
                        {
                            camera.renderTargetConfig.reset();
                        }
                    }

                    if (camera.renderTargetConfig)
                    {
                        ImGui::Indent();
                        int size[2] = { (int)camera.renderTargetConfig->width, (int)camera.renderTargetConfig->height };
                        if (ImGui::DragInt2("Resolution", size, 1, 16, 4096))
                        {
                            camera.renderTargetConfig->width = size[0];
                            camera.renderTargetConfig->height = size[1];
                        }
                        ImGui::Checkbox("Use Screen Space Aspect Ratio",
                                        &camera.renderTargetConfig->useScreenSpaceAspectRatio);
                        ImGui::Unindent();
                    }

                    ImGui::TreePop();
                }
            });

        // UIElement
        Register<UIElement>(
            [](Scene* scene, entt::entity e, const UIContext& ctx)
            {
                auto& element = scene->GetRegistry().get<UIElement>(e);
                bool removed = false;

                if (DebugUtils::DrawComponentHeader("UI Element", &removed))
                {
                    if (removed)
                    {
                        scene->GetRegistry().remove<UIElement>(e);
                        ImGui::TreePop();
                        return;
                    }

                    ImGui::Checkbox("Enabled", &element.isEnabled);
                    ImGui::DragInt("Priority", &element.priority, 1, 0, 1000);
                    ImGui::DragFloat4("Viewport", &element.viewport.x, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Rotation", &element.rotation, 0.1f, -360.0f, 360.0f);
                    ImGui::ColorEdit4("Color", element.color.values);
                    ImGui::Separator();

                    const char* contentTypes[] = { "Image", "Text", "Button" };
                    int currentType = static_cast<int>(element.content.index());

                    if (ImGui::Combo("Content Type", &currentType, contentTypes, IM_ARRAYSIZE(contentTypes)))
                    {
                        if (currentType != element.content.index())
                        {
                            switch (currentType)
                            {
                                case 0:
                                    element.content.emplace<UIImage>();
                                    break;
                                case 1:
                                    element.content.emplace<UIText>();
                                    break;
                                case 2:
                                    element.content.emplace<UIButton>();
                                    break;
                            }
                        }
                    }
                    ImGui::Separator();

                    auto DrawAssetPathControl = [](const char* label,
                                                   std::string& filepath,
                                                   const std::vector<std::string>& validExtensions) -> bool
                    {
                        bool changed = false;
                        char buffer[256];
                        strncpy_s(buffer, sizeof(buffer), filepath.c_str(), _TRUNCATE);

                        if (ImGui::InputText(label, buffer, sizeof(buffer)))
                        {
                            filepath = buffer;
                            changed = true;
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                const wchar_t* pathW = (const wchar_t*)payload->Data;
                                std::filesystem::path droppedPath(pathW);

                                std::string ext = droppedPath.extension().string();
                                if (std::find(validExtensions.begin(), validExtensions.end(), ext) !=
                                    validExtensions.end())
                                {
                                    filepath = droppedPath.string();
                                    changed = true;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }

                        return changed;
                    };

                    std::visit(
                        [&](auto& arg)
                        {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, UIImage>)
                            {
                                if (DrawAssetPathControl("Texture", arg.textureFilepath, IMAGE_FILE_EXTENSIONS))
                                {
                                    arg.SetTexturePath(arg.textureFilepath);
                                }
                            }
                            else if constexpr (std::is_same_v<T, UIText>)
                            {
                                char textBuffer[256];
                                strncpy_s(textBuffer, sizeof(textBuffer), arg.text.c_str(), _TRUNCATE);
                                if (ImGui::InputTextMultiline("Text", textBuffer, sizeof(textBuffer)))
                                {
                                    arg.text = textBuffer;
                                }
                                ImGui::DragFloat("Font Size", &arg.fontSize, 0.01f, 0.1f, 10.0f);

                                if (DrawAssetPathControl("Font", arg.fontFilepath, FONT_FILE_EXTENSIONS))
                                {
                                    arg.font = AssetManager::LoadAsset<Font>(arg.fontFilepath);
                                }
                            }
                            else if constexpr (std::is_same_v<T, UIButton>)
                            {
                                if (DrawAssetPathControl(
                                        "Idle Texture", arg.idleTextureFilepath, IMAGE_FILE_EXTENSIONS))
                                {
                                    TextureConfig config = { .textureType = TextureType::HUD,
                                                             .path = arg.idleTextureFilepath };
                                    arg.idleTexture = AssetManager::LoadAsset(arg.idleTextureFilepath, config);
                                }

                                if (DrawAssetPathControl(
                                        "Hover Texture", arg.hoverTextureFilepath, IMAGE_FILE_EXTENSIONS))
                                {
                                    TextureConfig config = { .textureType = TextureType::HUD,
                                                             .path = arg.hoverTextureFilepath };
                                    arg.hoverTexture = AssetManager::LoadAsset(arg.hoverTextureFilepath, config);
                                }

                                if (DrawAssetPathControl(
                                        "Pressed Texture", arg.pressedTextureFilepath, IMAGE_FILE_EXTENSIONS))
                                {
                                    TextureConfig config = { .textureType = TextureType::HUD,
                                                             .path = arg.pressedTextureFilepath };
                                    arg.pressedTexture = AssetManager::LoadAsset(arg.pressedTextureFilepath, config);
                                }
                            }
                        },
                        element.content);

                    ImGui::TreePop();
                }
            });
    }
} // namespace Frost