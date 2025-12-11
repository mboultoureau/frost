#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Utils/Math/Vector.h"

#include <string>

namespace Frost
{
    class FROST_API DebugUtils
    {
    public:
        static bool DrawComponentHeader(const char* name, bool* outRemoved = nullptr);
        static bool DrawVec3Control(const std::string& label,
                                    Math::Vector3& values,
                                    float resetValue = 0.0f,
                                    float columnWidth = 100.0f);
        static bool DrawQuaternionControl(const std::string& label,
                                          Math::Vector4& quaternion,
                                          float resetValue = 0.0f,
                                          float columnWidth = 100.0f);
    };
} // namespace Frost
