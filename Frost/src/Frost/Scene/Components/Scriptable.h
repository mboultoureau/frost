#pragma once

#include "Frost/Scene/ECS/Component.h"

#include <memory>
#include <vector>

namespace Frost
{
    class Script;
}

namespace Frost::Component
{
    struct Scriptable : public Component
    {
        std::vector<std::unique_ptr<Script>> _scripts;

        Scriptable();
        Scriptable(const Scriptable&) = delete;
        Scriptable& operator=(const Scriptable&) = delete;
        Scriptable(Scriptable&&);
        Scriptable& operator=(Scriptable&&);
        ~Scriptable();
    };
} // namespace Frost::Component