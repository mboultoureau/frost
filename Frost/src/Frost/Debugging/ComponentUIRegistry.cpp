#include "Frost/Debugging/ComponentUIRegistry.h"

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

                if (DebugUtils::DrawComponentHeader("Meta"))
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

                if (DebugUtils::DrawComponentHeader("Transform"))
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

                if (DebugUtils::DrawComponentHeader("Camera"))
                {
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
                if (DebugUtils::DrawComponentHeader("Static Mesh"))
                {
                    if (!ctx.isEditor)
                    {
                        ImGui::Text("Status: %s", (mesh.model ? "Loaded" : "Not Loaded"));
                    }
                    // TODO: Drag and drop mesh asset
                    if (ctx.isEditor)
                    {
                        ImGui::Button("Drag Mesh Here (TODO)");
                    }
                    ImGui::TreePop();
                }
            });
    }
} // namespace Frost