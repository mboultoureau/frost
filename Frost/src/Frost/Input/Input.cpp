#include "Input.h"

namespace Frost
{
	Input& Input::Get()
	{
		static Input singleton;
		return singleton;
	}

	Mouse& Input::GetMouse()
	{
		return Get()._mouse;
	}

	void Input::Update()
	{
		GetMouse().Update();
	}
}