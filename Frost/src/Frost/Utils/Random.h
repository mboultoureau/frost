#pragma once

#include "Frost.h"

#include <random>

namespace Frost
{
	class Random : NoCopy
	{
	public:
		Random() : prng{ rd() } {}

		static Random& Get()
		{
			static Random instance;
			return instance;
		}

		static std::mt19937& PRNG()
		{
			return Get().prng;
		}

	private:
		std::random_device rd;
		std::mt19937 prng;
	};
}


