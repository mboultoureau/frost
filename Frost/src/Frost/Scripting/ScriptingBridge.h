#pragma once

namespace Frost::Scripting
{
    // C struct to hold script names
    struct ScriptNameCollection
    {
        int count;
        const char** names;
    };
} // namespace Frost::Scripting
