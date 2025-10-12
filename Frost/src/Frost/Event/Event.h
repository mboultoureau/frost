#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Event/EventType.h"

#include <string>

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
}
