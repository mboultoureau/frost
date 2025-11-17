#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"

namespace Frost
{
	class DebugWindow : public DebugPanel
	{
	public:
	
		DebugWindow() = default;
		virtual ~DebugWindow() = default;
		virtual void OnImGuiRender(float deltaTime) override;
		virtual const char* GetName() const override { return "Debug Window"; }
	};
}