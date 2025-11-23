#pragma once
#include "Frost.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

#pragma once
#include "Frost/Scene/Components/Script.h"
#include <Frost/Utils/Math/Vector.h>

class Water
{
public:

	Water(Vector3 pos, EulerAngles rot, Vector3 scale, float waveAmplitude);
	JPH::RVec3 GetWaterSurfacePosition(JPH::RVec3Arg inXZPosition) const;
	JPH::BodyID GetBodyId() { return _bodyId; }

	float waveFrequency = 1.0f;
	float waveLength = .1f;

private:
	float cMaxWaterHeight = 2.0f;
	float cMinWaterHeight = 0.0f;
	GameObject _water;
	JPH::BodyID _bodyId;
	void _SetupPhysics();
};


class WaterScript : public Script {
	Water* _water;

public:
	WaterScript(Water* water) : _water{ water }
	{
	}

	void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override;

	void OnCollisionStay(BodyOnContactParameters params, float deltaTime) override;

#if FT_DEBUG;
	void OnFixedUpdate(float fixedDeltaTime) override;
#endif
	void OnLateUpdate(float deltaTime) override;

	// Configuration
	static constexpr float		cBoatBuoyancy = 3.0f;
	static constexpr float		cBoatLinearDrag = 0.04f;
	static constexpr float		cBoatAngularDrag = 0.7f;

	static constexpr float		cBarrelMass = 50.0f;
	static constexpr float		cBarrelBuoyancy = 1.5f;
	static constexpr float		cBarrelLinearDrag = 0.5f;
	static constexpr float		cBarrelAngularDrag = 0.1f;
};