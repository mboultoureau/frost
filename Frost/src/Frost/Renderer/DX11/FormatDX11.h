#pragma once

#include "Frost/Renderer/Format.h"
#include <dxgiformat.h>
#include <cstdint>

namespace Frost
{
	DXGI_FORMAT ToDXGIFormat(Format format);
	uint32_t GetFormatSize(Format format);
}