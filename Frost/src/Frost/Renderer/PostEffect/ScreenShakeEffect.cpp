#include "Frost/Renderer/PostEffect/ScreenShakeEffect.h"

#include <random>
#include <vector>
#include <string>
#include <imgui.h>

static std::mt19937 gen(std::random_device{}());
static std::uniform_real_distribution<float> distr(-1.0f, 1.0f);

namespace Frost
{
	void ScreenShakeEffect::OnPreRender(float deltaTime, Math::Matrix4x4& viewMatrix, Math::Matrix4x4& projectionMatrix)
	{
		if (_currentTime > 0.0f)
		{
			_currentTime -= deltaTime;

			if (_currentTime <= 0.0f)
			{
				_currentTime = 0.0f;
				return;
			}

			// Calculate attenuation
			float progress = 1.0f - (_currentTime / _duration);
			float multiplier = 1.0f;

			switch (_attenuationType)
			{
			case AttenuationType::Linear:
				multiplier = 1.0f - progress;
				break;
			case AttenuationType::EaseOut:
				multiplier = (1.0f - progress) * (1.0f - progress);
				break;
			case AttenuationType::EaseIn:
				multiplier = progress * progress;
				break;
			case AttenuationType::Constant:
				multiplier = 1.0f;
				break;
			}

			float currentAmplitude = _amplitude * multiplier;

			float offsetX = distr(gen) * currentAmplitude;
			float offsetY = distr(gen) * currentAmplitude;

			viewMatrix = Math::Matrix4x4::CreateTranslation({ offsetX, offsetY, 0.0f }) * viewMatrix;
		}
	}

	void ScreenShakeEffect::OnImGuiRender(float deltaTime)
	{
		// Current State
		ImGui::Text("Current State");
		ImGui::Separator();

		const char* currentAttenuationName = "None";
		if (_currentTime > 0.0f)
		{
			switch (_attenuationType)
			{
				case AttenuationType::Linear:   currentAttenuationName = "Linear";   break;
				case AttenuationType::EaseOut:  currentAttenuationName = "EaseOut";  break;
				case AttenuationType::EaseIn:    currentAttenuationName = "EaseIn";  break;
				case AttenuationType::Constant: currentAttenuationName = "Constant"; break;
			}
		}
		ImGui::Text("Time Remaining: %.2f s", _currentTime);
		ImGui::Text("Current Amplitude: %.2f", _amplitude * 1.0f);
		ImGui::Text("Attenuation Type: %s", currentAttenuationName);

		ImGui::Spacing();
		ImGui::Spacing();

		// Testing Shake with Shake function
		ImGui::Text("Shake Controls");
		ImGui::Separator();
		static float shakeDuration = 1.0f;
		static float shakeAmplitude = 5.0f;
		static int selectedAttenuation = 1;

		ImGui::SliderFloat("Duration##Shake", &shakeDuration, 0.1f, 5.0f, "%.2f s");
		ImGui::SliderFloat("Amplitude##Shake", &shakeAmplitude, 0.1f, 20.0f, "%.2f");

		const char* attenuationItems[] = { "Linear", "EaseOut", "EaseIn", "Constant" };
		ImGui::Combo("Attenuation", &selectedAttenuation, attenuationItems, IM_ARRAYSIZE(attenuationItems));

		if (ImGui::Button("Shake!", ImVec2(-1, 0)))
		{
			Shake(shakeDuration, shakeAmplitude, static_cast<AttenuationType>(selectedAttenuation));
		}

		ImGui::Spacing();
		ImGui::Spacing();

		// Visualisation
		ImGui::Text("Visualization");
		ImGui::Separator();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		ImVec2 canvasSize = ImGui::GetContentRegionAvail();
		if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
		if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;
		canvasSize.y = 100;

		drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(30, 30, 30, 255));
		drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + canvasSize.y / 2), ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y / 2), IM_COL32(80, 80, 80, 255));

		if (_currentTime > 0.0f)
		{
			std::vector<ImVec2> points;
			for (int i = 0; i < canvasSize.x; ++i)
			{
				float t = (float)i / (canvasSize.x - 1);
				float multiplier = 1.0f;

				switch (_attenuationType)
				{
					case AttenuationType::Linear:   multiplier = 1.0f - t;					break;
					case AttenuationType::EaseOut:  multiplier = (1.0f - t) * (1.0f - t);	break;
					case AttenuationType::EaseIn:   multiplier = t * t;						break;
					case AttenuationType::Constant: multiplier = 1.0f;						break;
				}

				float vizAmplitude = _amplitude * multiplier;

				float yOffset = sin(t * 40.0f) * vizAmplitude * 2.0f;
				points.push_back(ImVec2(canvasPos.x + i, canvasPos.y + canvasSize.y / 2 - yOffset));
			}
			drawList->AddPolyline(points.data(), points.size(), IM_COL32(255, 100, 100, 255), false, 1.5f);
		}
		ImGui::Dummy(canvasSize);
	}

	void ScreenShakeEffect::Shake(float duration, float amplitude, AttenuationType type)
	{
		_duration = duration > 0.0f ? duration : 0.01f;
		_currentTime = _duration;
		_amplitude = amplitude;
		_attenuationType = type;
	}
}