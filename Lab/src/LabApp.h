#pragma once

#include "Game.h"

#include "Frost/Core/Application.h"

#include <memory>

class Lab : public Frost::Application
{
public:
    Lab(Frost::ApplicationEntryPoint entryPoint);
    ~Lab();

    void OnApplicationReady() override;

private:
    std::unique_ptr<Game> _game;
    Frost::PhysicsConfig _physicsConfig;
};