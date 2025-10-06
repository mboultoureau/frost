#pragma once

#include "Core/Core.h"

class FT_API NoCopy
{
public:
	NoCopy(const NoCopy&) = delete;
	NoCopy& operator=(const NoCopy&) = delete;
protected:
	constexpr NoCopy() = default;
	~NoCopy() = default;
};