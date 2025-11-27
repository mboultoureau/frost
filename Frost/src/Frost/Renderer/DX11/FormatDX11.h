#pragma once

#include "Frost/Renderer/Format.h"
#include <cstdint>
#include <dxgiformat.h>

namespace Frost
{
    DXGI_FORMAT
    ToDXGIFormat(Format format);
    uint32_t GetFormatSize(Format format);
} // namespace Frost