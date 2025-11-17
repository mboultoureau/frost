#pragma once

namespace Frost
{
	class DebugPanel
	{
	public:
		DebugPanel() = default;
		virtual ~DebugPanel() = default;
		virtual void OnImGuiRender(float deltaTime) = 0;

		virtual const char* GetName() const = 0;
	};
}