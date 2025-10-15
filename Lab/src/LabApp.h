#pragma once

#include "Game.h"

#include "Frost/Core/Application.h"

#include <memory>

class Lab : public Frost::Application
{
public:
	Lab(Frost::ApplicationEntryPoint entryPoint);
	~Lab();

private:
	std::unique_ptr<Game> _game;
};