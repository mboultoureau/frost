#pragma once

#include "Frost/Input/Devices/Mouse.h"
#include "Frost/Utils/NoCopy.h"

namespace Frost
{
	class Input : NoCopy
	{
	public:
		static Input& Get();
		static Mouse& GetMouse();

		static void Update();

	private:
		Input() = default;

	private:
		Mouse _mouse;
	};
}
