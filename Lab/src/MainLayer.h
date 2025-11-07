#pragma once

#include "Objects/Sphere.h"
#include "Objects/FreeCamera.h"
#include "Objects/Moto.h"
#include "Objects/TopCamera.h"
#include "Objects/Plane.h"
#include "Objects/Ogre.h"
#include "Objects/OgreNormal.h"
#include "Objects/Text.h"
#include "Objects/HUD_Logo.h"
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
	std::unique_ptr<Ogre> _ogre;
	std::unique_ptr<OgreNormal> _ogreNormal;
	std::unique_ptr<Text> _text;
	std::unique_ptr<HUD_Logo> _hudLogo;
};

