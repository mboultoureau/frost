#pragma once

#include <entt/entt.hpp>
#include "Frost/Utils/Math/Matrix.h"

namespace Frost::Component
{
	struct RelativeView
	{
		entt::entity referenceEntity;
		Math::Matrix4x4 modifier = Math::Matrix4x4::CreateIdentity();
	};
}
