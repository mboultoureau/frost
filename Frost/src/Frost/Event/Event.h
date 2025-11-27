#pragma once

#include "Frost/Event/EventType.h"
#include "Frost/Utils/NoCopy.h"

#include <string>

#define FROST_BIND_EVENT_FN(fn) [this](auto& e) { return this->fn(e); }

namespace Frost
{
    class Event : NoCopy
    {
    public:
        virtual EventType GetEventType() const = 0;
        bool IsHandled() const { return _handled; }
        void Handle() { _handled = true; }

        virtual std::string ToString() const = 0;

    private:
        bool _handled = false;
    };
} // namespace Frost
