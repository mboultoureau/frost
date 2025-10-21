#pragma once

#include <chrono>

namespace Frost
{
	


	class Timer
	{
	public:
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = Clock::time_point;
		using Duration = Clock::duration;

		Timer();

		void Start();
		void Pause();
		void Resume();
		
		Duration GetDuration();

		template<typename DurationT>
		DurationT GetDurationAs() const
		{
			return std::chrono::duration_cast<DurationT>(GetDuration());
		}

	private:
		Duration _duration = Duration::zero();
		TimePoint _lastTime;
		bool _running;
	};
}

