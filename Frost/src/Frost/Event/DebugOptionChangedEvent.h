#pragma once

#include "Frost/Event/Event.h"

#include <string>
#include <variant>

namespace Frost
{
	class DebugOptionChangedEvent : public Event
	{
	public:
		enum Options
		{
			NONE = 0,
			SHOW_WIREFRAME,
			_COUNT
		};

		using OptionType = Options;
		using NewValueType = std::variant<int, float, bool, std::string>;

		DebugOptionChangedEvent(OptionType _optionType, const NewValueType& newValue)
			: _optionType{ _optionType }, _newValue(newValue) {
		}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "DebugOptionChangedEvent"; }

		static EventType GetStaticType() { return EventType::DebugOptionChanged; }

		const OptionType& GetOptionType() const { return _optionType; }
		const NewValueType& GetNewValue() const { return _newValue; }

	private:
		OptionType _optionType;
		NewValueType _newValue;
	};
}
