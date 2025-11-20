#include "Frost/Debugging/DebugInterface/DebugScene.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/HUDImage.h"
#include "Frost/Scene/Components/UIButton.h"
#include "Frost/Debugging/DebugInterface/DebugUtils.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Physics/Physics.h"

#include <imgui.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Frost::Component;

namespace Frost
{
	void DebugScene::OnImGuiRender(float deltaTime)
	{
		_deltaTime = deltaTime;

		if (ImGui::CollapsingHeader("Scenes"))
		{
			if (_scenes.empty())
			{
				ImGui::Text("No scene attached to DebugLayer");
			}
		
			for (Scene* scene : _scenes)
			{
				if (ImGui::TreeNode(scene->GetName().c_str()))
				{
					for (GameObject::Id gameObjectId : scene->GetECS().GetActiveGameObjects())
					{
						Meta* info = scene->GetECS().GetComponent<Meta>(gameObjectId);
		
						static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
						ImGuiTreeNodeFlags node_flags = base_flags;
		
						//bool is_root = (info && info->parentId == GameObject::InvalidId);
						bool is_root = true;
						if (!is_root) node_flags |= ImGuiTreeNodeFlags_Leaf;
		
						const bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)gameObjectId, node_flags, "%s (ID: %llu)", info->name.c_str(), gameObjectId);
		
						if (is_open)
						{
							// Common
							_DrawMetaComponent(scene, gameObjectId);
							_DrawTransformComponent(scene, gameObjectId);
							_DrawWorldTransformComponent(scene, gameObjectId);
							
							// Specific
							_DrawCameraComponent(scene, gameObjectId);
							_DrawLightComponent(scene, gameObjectId);
							_DrawStaticMeshComponent(scene, gameObjectId);
							_DrawRigidBodyComponent(scene, gameObjectId);
							_DrawScriptableComponent(scene, gameObjectId);
							_DrawHUDImageComponent(scene, gameObjectId);
							_DrawUIButtonComponent(scene, gameObjectId);
		
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
		}
	}

	void DebugScene::AddScene(Scene* scene)
	{
		_scenes.push_back(scene);
	}

	void DebugScene::RemoveScene(Scene* scene)
	{
		_scenes.erase(std::remove(_scenes.begin(), _scenes.end(), scene), _scenes.end());
	}

	void DebugScene::_DrawMetaComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		Meta* info = scene->GetECS().GetComponent<Meta>(gameObjectId);
		if (!info) return;

		if (DebugUtils::DrawComponentHeader("Meta"))
		{
			ImGui::Text("ID: %llu", info->id);
			ImGui::Text("Parent ID: %llu", info->parentId);
			char nameBuffer[256];
			strncpy_s(nameBuffer, sizeof(nameBuffer), info->name.c_str(), sizeof(nameBuffer) - 1);
			nameBuffer[sizeof(nameBuffer) - 1] = '\0';
			if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
			{
				info->name = std::string(nameBuffer);
			}
			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawTransformComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		Transform* transform = scene->GetECS().GetComponent<Transform>(gameObjectId);
		if (!transform) return;

		if (DebugUtils::DrawComponentHeader("Transform (Local)"))
		{
			DebugUtils::DrawVec3Control("Position", transform->position);
			DebugUtils::DrawQuaternionControl("Rotation", transform->rotation);
			DebugUtils::DrawVec3Control("Scale", transform->scale);
			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawWorldTransformComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		WorldTransform* worldTransform = scene->GetECS().GetComponent<WorldTransform>(gameObjectId);
		if (!worldTransform) return;

		if (DebugUtils::DrawComponentHeader("World Transform (Read-Only)"))
		{
			Math::EulerAngles eulerAnglesRadians = Math::QuaternionToEulerAngles(worldTransform->rotation);
			Math::Vector3 eulerAnglesDegrees{
				Math::Angle<Math::Degree>(eulerAnglesRadians.Pitch).value(),
				Math::Angle<Math::Degree>(eulerAnglesRadians.Yaw).value(),
				Math::Angle<Math::Degree>(eulerAnglesRadians.Roll).value()
			};

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", worldTransform->position.x, worldTransform->position.y, worldTransform->position.z);
			ImGui::Text("Rotation (Quat): (%.2f, %.2f, %.2f, %.2f)", worldTransform->rotation.x, worldTransform->rotation.y, worldTransform->rotation.z, worldTransform->rotation.w);
			ImGui::Text("Rotation (Euler): (P:%.2f, Y:%.2f, R:%.2f) deg", eulerAnglesDegrees.x, eulerAnglesDegrees.y, eulerAnglesDegrees.z);
			ImGui::Text("Scale:    (%.2f, %.2f, %.2f)", worldTransform->scale.x, worldTransform->scale.y, worldTransform->scale.z);

			ImGui::PopStyleColor();
			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawStaticMeshComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		StaticMesh* staticMesh = scene->GetECS().GetComponent<StaticMesh>(gameObjectId);
		if (!staticMesh) return;

		if (DebugUtils::DrawComponentHeader("Mesh Renderer"))
		{
			//ImGui::Text("Filepath: %s", staticMesh->modelFilepath.c_str());
			ImGui::Text("Mesh Status: %s", (staticMesh->model ? "Loaded" : "Not Loaded (Error or null)"));
			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawCameraComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		Camera* camera = scene->GetECS().GetComponent<Camera>(gameObjectId);
		if (!camera) return;

		if (DebugUtils::DrawComponentHeader("Camera"))
		{
			const char* projectionTypes[] = { "Perspective", "Orthographic" };
			int currentType = (int)camera->projectionType;
			if (ImGui::Combo("Projection Type", &currentType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
			{
				camera->projectionType = (Camera::ProjectionType)currentType;
			}

			if (camera->projectionType == Camera::ProjectionType::Perspective)
			{
				float currentFOVDegrees = Math::Angle<Math::Degree>(camera->perspectiveFOV).value();
				ImGui::DragFloat("FOV", &currentFOVDegrees, 0.5f, 30.0f, 120.0f);
				camera->perspectiveFOV = Math::Angle<Math::Radian>{
					Math::angle_cast<Math::Radian, Math::Degree>(currentFOVDegrees)
				};
			}
			else
			{
				ImGui::DragFloat("Size", &camera->orthographicSize, 0.1f, 1.0f, 1000.0f);
			}

			ImGui::Separator();
			ImGui::DragFloat("Near Clip", &camera->nearClip, 0.01f, 0.001f, camera->farClip - 0.1f, "%.3f");
			ImGui::DragFloat("Far Clip", &camera->farClip, 1.0f, camera->nearClip + 0.1f, 10000.0f);

			ImGui::Separator();
			ImGui::Checkbox("Clear On Render", &camera->clearOnRender);
			ImGui::ColorEdit4("Background Color", camera->backgroundColor.values, ImGuiColorEditFlags_NoAlpha);

			ImGui::Text("Viewport");
			ImGui::SameLine();

			float totalAvailableWidth = ImGui::GetContentRegionAvail().x;
			float itemWidth = (totalAvailableWidth - ImGui::GetStyle().ItemSpacing.x * 3) / 4;

			ImGui::PushItemWidth(itemWidth);
			if (ImGui::DragFloat("##X", &camera->viewport.x, 0.01f, 0.0f, 1.0f, "X:%.2f")) {} ImGui::SameLine();
			if (ImGui::DragFloat("##Y", &camera->viewport.y, 0.01f, 0.0f, 1.0f, "Y:%.2f")) {} ImGui::SameLine();
			if (ImGui::DragFloat("##W", &camera->viewport.width, 0.01f, 0.001f, 1.0f, "W:%.2f")) {} ImGui::SameLine();
			if (ImGui::DragFloat("##H", &camera->viewport.height, 0.01f, 0.001f, 1.0f, "H:%.2f")) {}
			ImGui::PopItemWidth();

			// Post effects
			ImGui::Separator();
			ImGui::Text("Post Effects:");

			for (size_t i = 0; i < camera->postEffects.size(); ++i)
			{
				PostEffect* effect = camera->postEffects[i].get();
				if (!effect) continue;
				ImGui::PushID((int)i);
				bool enabled = effect->IsEnabled();
				if (ImGui::Checkbox("##enabled", &enabled))
				{
					effect->SetEnabled(enabled);
				}
				ImGui::SameLine();
				if (ImGui::CollapsingHeader(effect->GetName()))
				{
					ImGui::Indent();
					effect->OnImGuiRender(_deltaTime);
					ImGui::Unindent();
				}
				ImGui::PopID();
			}

			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawScriptableComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		Scriptable* scriptable = scene->GetECS().GetComponent<Scriptable>(gameObjectId);
		if (!scriptable) return;

		if (DebugUtils::DrawComponentHeader("Scriptable"))
		{
			ImGui::Text("Script Count: %llu", scriptable->_scripts.size());
			ImGui::SameLine();
			if (ImGui::Button("Clear All Scripts"))
			{
				scriptable->_scripts.clear();
			}

			ImGui::Separator();

			for (long long i = scriptable->_scripts.size() - 1; i >= 0; --i)
			{
				const auto& scriptPtr = scriptable->_scripts[i];
				char label[64];
				snprintf(label, sizeof(label), "Script %llu", i);
				ImGui::PushID((int)i);
				ImGui::Text("Script Index: %llu", i);
				ImGui::SameLine();

				if (ImGui::Button("Delete"))
				{
					scriptable->_scripts.erase(scriptable->_scripts.begin() + i);
					ImGui::PopID();
					break;
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawLightComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		Light* light = scene->GetECS().GetComponent<Light>(gameObjectId);
		if (!light) return;

		const char* lightTypeName = "Unknown";
		switch (light->type)
		{
		case LightType::Directional: lightTypeName = "Directional"; break;
		case LightType::Point:       lightTypeName = "Point";       break;
		case LightType::Spot:        lightTypeName = "Spot";        break;
		}
		char headerLabel[64];
		snprintf(headerLabel, sizeof(headerLabel), "Light (%s)", lightTypeName);

		if (DebugUtils::DrawComponentHeader(headerLabel))
		{
			const char* lightTypes[] = { "Directional", "Point", "Spot" };
			int currentType = static_cast<int>(light->type);
			if (ImGui::Combo("Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes)))
			{
				light->type = static_cast<LightType>(currentType);
			}

			ImGui::Separator();

			ImGui::ColorEdit3("Color", &light->color.x, ImGuiColorEditFlags_Float);
			ImGui::DragFloat("Intensity", &light->intensity, 0.05f, 0.0f, FLT_MAX);

			if (light->type == LightType::Point || light->type == LightType::Spot)
			{
				ImGui::DragFloat("Radius", &light->radius, 0.1f, 0.01f, 10000.0f);
			}

			if (light->type == LightType::Spot)
			{
				ImGui::Text("Spot Angles");

				float outerAngleDegrees = Math::Angle<Math::Degree>(light->outerConeAngle).value();

				if (ImGui::SliderFloat("Outer Cone", &outerAngleDegrees, 1.0f, 90.0f, "%.1f deg"))
				{
					light->outerConeAngle = Math::Angle<Math::Degree>(outerAngleDegrees);
				}

				float innerAngleDegrees = Math::Angle<Math::Degree>(light->innerConeAngle).value();

				if (light->innerConeAngle > light->outerConeAngle)
				{
					light->innerConeAngle = light->outerConeAngle;
					innerAngleDegrees = Math::Angle<Math::Degree>(light->innerConeAngle).value();
				}

				if (ImGui::SliderFloat("Inner Cone", &innerAngleDegrees, 0.0f, outerAngleDegrees, "%.1f deg"))
				{
					light->innerConeAngle = Math::Angle<Math::Degree>(innerAngleDegrees);
				}
			}

			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawRigidBodyComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		RigidBody* rigidBody = scene->GetECS().GetComponent<RigidBody>(gameObjectId);
		if (!rigidBody) return;

		if (DebugUtils::DrawComponentHeader("Rigid Body (Jolt)"))
		{
			ImGui::Text("Body ID: %u", rigidBody->physicBody->bodyId.GetIndex());

			const JPH::BodyLockInterface& bodyLockInterface = Physics::Get().GetBodyLockInterface();
			JPH::BodyLockRead lock(bodyLockInterface, rigidBody->physicBody->bodyId);
			if (lock.Succeeded())
			{
				const JPH::Body& body = lock.GetBody();

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

				const char* motionTypes[] = { "Static", "Kinematic", "Dynamic" };
				ImGui::Text("Motion Type: %s", motionTypes[(int)body.GetMotionType()]);
				ImGui::Text("Is Active: %s", body.IsActive() ? "True" : "False");
				ImGui::Text("Is Sensor: %s", body.IsSensor() ? "True" : "False");
				ImGui::PopStyleColor();

				if (body.GetMotionType() != JPH::EMotionType::Static)
				{
					JPH::Vec3 linearVelocity = body.GetLinearVelocity();
					float linearVel[3] = { linearVelocity.GetX(), linearVelocity.GetY(), linearVelocity.GetZ() };

					if (ImGui::DragFloat3("Linear Velocity", linearVel, 0.05f))
					{
						// Physics::GetBodyInterface().SetLinearVelocity(rigidBody->bodyId, JPH::Vec3(linearVel[0], linearVel[1], linearVel[2]));
					}

					JPH::Vec3 angularVelocity = body.GetAngularVelocity();
					float angularVel[3] = { angularVelocity.GetX(), angularVelocity.GetY(), angularVelocity.GetZ() };
					if (ImGui::DragFloat3("Angular Velocity", angularVel, 0.05f))
					{
						// Physics::GetBodyInterface().SetAngularVelocity(rigidBody->bodyId, JPH::Vec3(angularVel[0], angularVel[1], angularVel[2]));
					}

					float gravity = body.GetBodyCreationSettings().mGravityFactor;
					if (ImGui::DragFloat("Gravity Factor", &gravity, 0.01, 0.0f, 10.0f))
					{
						// Physics::GetBodyInterface().SetGravityFactor(rigidBody->bodyId, gravity);
					}
				}

			}
			else
			{
				ImGui::Text("Body Lock Failed (Body might be removed)");
			}


			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawHUDImageComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		if (scene->GetECS().GetComponent<UIButton>(gameObjectId)) return;

		HUDImage* image = scene->GetECS().GetComponent<HUDImage>(gameObjectId);
		if (!image) return;

		if (DebugUtils::DrawComponentHeader("HUD Image"))
		{
			ImGui::Checkbox("Enabled", &image->isEnabled);
			ImGui::Separator();

			ImGui::Text("Texture: %s", image->textureFilepath.c_str());
			ImGui::Text("Status: %s", (image->texture ? "Loaded" : "Not Loaded"));

			const char* filterModes[] = { "Point", "Linear", "Anisotropic" };
			int currentFilter = static_cast<int>(image->textureFilter);
			if (ImGui::Combo("Filter Mode", &currentFilter, filterModes, IM_ARRAYSIZE(filterModes)))
			{
				image->textureFilter = static_cast<Material::FilterMode>(currentFilter);
			}

			ImGui::Separator();
			ImGui::Text("Viewport");
			ImGui::SameLine();

			float totalAvailableWidth = ImGui::GetContentRegionAvail().x;
			float itemWidth = (totalAvailableWidth - ImGui::GetStyle().ItemSpacing.x * 3) / 4;

			ImGui::PushItemWidth(itemWidth);
			ImGui::DragFloat("##X", &image->viewport.x, 0.01f, 0.0f, 1.0f, "X:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##Y", &image->viewport.y, 0.01f, 0.0f, 1.0f, "Y:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##W", &image->viewport.width, 0.01f, 0.001f, 1.0f, "W:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##H", &image->viewport.height, 0.01f, 0.001f, 1.0f, "H:%.2f");
			ImGui::PopItemWidth();

			ImGui::TreePop();
		}
	}

	void DebugScene::_DrawUIButtonComponent(Scene* scene, GameObject::Id gameObjectId)
	{
		UIButton* button = scene->GetECS().GetComponent<UIButton>(gameObjectId);
		if (!button) return;

		if (DebugUtils::DrawComponentHeader("UI Button"))
		{
			ImGui::Checkbox("Enabled", &button->isEnabled);
			ImGui::Separator();

			ImGui::Text("Idle Texture: %s", button->textureFilepath.c_str());
			ImGui::Text("  Status: %s", (button->idleTexture ? "Loaded" : "Not Loaded"));

			ImGui::Text("Hover Texture: %s", button->hoverTextureFilepath.c_str());
			ImGui::Text("  Status: %s", (button->hoverTexture ? "Loaded" : "Not Loaded"));

			ImGui::Text("Pressed Texture: %s", button->pressedTextureFilepath.c_str());
			ImGui::Text("  Status: %s", (button->pressedTexture ? "Loaded" : "Not Loaded"));

			ImGui::Separator();

			ImGui::Text("Render Viewport");
			float totalAvailableWidth = ImGui::GetContentRegionAvail().x;
			float itemWidth = (totalAvailableWidth - ImGui::GetStyle().ItemSpacing.x * 3) / 4;

			ImGui::PushItemWidth(itemWidth);
			ImGui::DragFloat("##VX", &button->viewport.x, 0.01f, 0.0f, 1.0f, "X:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##VY", &button->viewport.y, 0.01f, 0.0f, 1.0f, "Y:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##VW", &button->viewport.width, 0.01f, 0.001f, 1.0f, "W:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##VH", &button->viewport.height, 0.01f, 0.001f, 1.0f, "H:%.2f");
			ImGui::PopItemWidth();

			ImGui::Text("Button Hitbox");
			ImGui::PushItemWidth(itemWidth);
			ImGui::DragFloat("##HX", &button->buttonHitbox.x, 0.01f, 0.0f, 1.0f, "X:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##HY", &button->buttonHitbox.y, 0.01f, 0.0f, 1.0f, "Y:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##HW", &button->buttonHitbox.width, 0.01f, 0.001f, 1.0f, "W:%.2f"); ImGui::SameLine();
			ImGui::DragFloat("##HH", &button->buttonHitbox.height, 0.01f, 0.001f, 1.0f, "H:%.2f");
			ImGui::PopItemWidth();

			ImGui::Separator();
			ImGui::Text("onClick Callback: %s", (button->onClick ? "Attached" : "Not Attached"));

			ImGui::TreePop();
		}
	}
}
