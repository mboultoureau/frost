#pragma once

#ifdef FT_PLATFORM_WINDOWS
#else
	#error Frost only supports Windows!
#endif

// Enable Jolt Debug Renderer
#ifdef FT_DEBUG
	#define JPH_DEBUG_RENDERER
#endif