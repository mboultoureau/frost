#pragma once

#ifdef FT_PLATFORM_WINDOWS

#include "Frost/Core/Application.h"

#include <Windows.h>
#include <iostream>

extern Frost::Application*
Frost::CreateApplication(Frost::ApplicationSpecification entryPoint);

int WINAPI
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    std::cout << "Frost Engine (version 0.0.1)" << std::endl;

    Frost::ApplicationSpecification entryPoint{};

    auto application = Frost::CreateApplication(entryPoint);
    application->Setup();
    application->OnApplicationReady();
    application->Run();
    delete application;

    return 0;
}

#endif