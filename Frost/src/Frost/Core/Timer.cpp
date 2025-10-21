#include "Frost/Core/Timer.h"

namespace Frost
{
	Timer::Timer() : _running{ false }
	{
	}

	void Timer::Start()
	{
		_running = true;
		_duration = Duration::zero();
		_lastTime = Clock::now();
	}

	void Timer::Pause()
	{
		_running = false;
		_duration += Clock::now() - _lastTime;
	}

	void Timer::Resume()
	{
		if (_running)
		{
			return;
		}

		_running = true;
		_lastTime = Clock::now();
	}

	Timer::Duration Timer::GetDuration()
	{
		return Clock::now() - _lastTime + _duration;
	}
}
