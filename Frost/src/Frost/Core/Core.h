#pragma once

#ifdef FT_PLATFORM_WINDOWS
#ifdef FT_BUILD_DLL
#define FROST_API __declspec(dllexport)
#else
#define FROST_API __declspec(dllimport)
#endif
#else
#define FROST_API
#error Frost only supports Windows!
#endif