#pragma once

#include "Objects/Sphere.h"
#include "Objects/FreeCamera.h"
#include "Objects/Moto.h"
#include "Objects/TopCamera.h"
#include "Objects/Plane.h"

#include <memory>

class MainLayer : public Frost::Layer
{
public:
	MainLayer();

	void OnLateUpdate(float deltaTime) override;
	void OnUpdate(float deltaTime) override;
	void OnFixedUpdate(float deltaTime) override;
	void OnAttach() override;

private:
	std::unique_ptr<Sphere> _sphere;
	std::unique_ptr<FreeCamera> _freeCamera;
	std::unique_ptr<TopCamera> _topCamera;
	std::unique_ptr<Moto> _moto;
	std::unique_ptr<Plane> _plane;
};

