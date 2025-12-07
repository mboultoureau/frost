#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scripting/Script.h"

namespace Frost::Component
{
    Scriptable::Scriptable() = default;
    Scriptable::Scriptable(Scriptable&&) = default;
    Scriptable& Scriptable::operator=(Scriptable&&) = default;
    Scriptable::~Scriptable() = default;
} // namespace Frost::Component
