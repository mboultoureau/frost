#pragma once

#include "Frost/Utils/Math/Matrix.h"

namespace Frost
{
	class PostEffect
	{
	public:
		virtual ~PostEffect() = default;
		
		virtual void OnPreRender(float deltaTime, Math::Matrix4x4& viewMatrix, Math::Matrix4x4& projectionMatrix) {};
		virtual void OnRender(float deltaTime) {};
		virtual void OnImGuiRender(float deltaTime) {};
		virtual const char* GetName() const { return "PostEffect"; }

		bool IsEnabled() const { return _enabled; }
		void SetEnabled(bool enabled) { _enabled = enabled; }

	protected:
		bool _enabled = true;
	};
}