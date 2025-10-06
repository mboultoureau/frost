#pragma once

#ifdef FT_PLATFORM_WINDOWS
	#ifdef FT_BUILD_DLL
		#define FT_API __declspec(dllexport)
	#else
		#define FT_API __declspec(dllimport)
	#endif
#else
	#error Frost only supports Windows!
#endif