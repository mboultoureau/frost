#pragma once
#include "Frost/Debugging/Logger.h"
#include <Jolt/Physics/Body/Body.h>

namespace Frost
{

	struct PhysicBody
	{
		PhysicBody(JPH::BodyCreationSettings& bodySettings, GameObject::Id id, JPH::EActivation activationMode) :
			bodyId{ Physics::CreateAndAddBody(bodySettings, id, activationMode) }
		{
			Physics::Get().mapJBodyGameObject.insert({ bodyId, id });
		};

		~PhysicBody() {
			if (bodyId.IsInvalid() == false)
			{
				if (Physics::Get().body_interface->IsAdded(bodyId))
				{
					Physics::Get().body_interface->RemoveBody(bodyId);
				}
				Physics::Get().body_interface->DestroyBody(bodyId);
			}
		}

		JPH::BodyID bodyId;
	};

	struct RigidBody
	{
		RigidBody(JPH::BodyCreationSettings& bodySettings, GameObject::Id id, JPH::EActivation activationMode) : 
			physicBody{ std::make_shared<PhysicBody>(bodySettings, id, activationMode) }
		{
		};

		std::shared_ptr<PhysicBody> physicBody;
	};


}