#pragma once

#include "Frost/Event/Event.h"

#include <string>
#include <variant>

namespace Frost
{
	class DebugOptionChangedEvent : public Event
	{
	public:
		DebugOptionChangedEvent(const std::string& optionName, const std::variant<int, float, bool, std::string>& newValue)
			: _optionName(optionName), _newValue(newValue) {
		}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "DebugOptionChangedEvent"; }

		static EventType GetStaticType() { return EventType::DebugOptionChanged; }

		const std::string& GetOptionName() const { return _optionName; }
		const std::variant<int, float, bool, std::string>& GetNewValue() const { return _newValue; }

	private:
		std::string _optionName;
		std::variant<int, float, bool, std::string> _newValue;
	};
}
