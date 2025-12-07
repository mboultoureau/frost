#pragma once

#include "Frost/Scene/ECS/Component.h"

#include <memory>
#include <vector>
#include <string>

namespace Frost::Scripting
{
    class Script;
}

namespace Frost::Component
{
    struct Scriptable : public Component
    {
        std::vector<std::string> scriptNames;
        std::vector<std::unique_ptr<Frost::Scripting::Script>> _scripts;

        Scriptable();
        Scriptable(const Scriptable&) = delete;
        Scriptable& operator=(const Scriptable&) = delete;
        Scriptable(Scriptable&&);
        Scriptable& operator=(Scriptable&&);
        ~Scriptable();
    };
} // namespace Frost::Component