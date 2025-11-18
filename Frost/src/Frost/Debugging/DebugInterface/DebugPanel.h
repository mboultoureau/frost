#pragma once

namespace Frost
{
	class DebugPanel
	{
	public:
		DebugPanel() = default;
		virtual ~DebugPanel() = default;
		virtual void OnImGuiRender(float deltaTime) = 0;
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnLateUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}

		virtual const char* GetName() const = 0;
	};
}