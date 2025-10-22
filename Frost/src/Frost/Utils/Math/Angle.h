#pragma once

#include <string_view>
#include <numbers>

#include "Frost/Utils/Math/Approximate.h"

/**
* To learn more about changing the referential system, visit:
* @see https://h-deb.ca/Sujets/Divers--cplusplus/Implementer-changement-referentiel.html
*/

namespace Frost
{
	using namespace std::string_view_literals;

	class Degree {};
	class Radian {};

	struct base_angle_traits
	{
		using value_type = float;
	};

	template<class T>
	struct angle_traits;

	template<>
	struct angle_traits<Radian> : base_angle_traits
	{
		static constexpr value_type to_neutral(value_type value) noexcept
		{
			return value;
		}

		static constexpr value_type from_neutral(value_type value) noexcept
		{
			return value;
		}

		static constexpr auto name() noexcept
		{
			return "Radian"sv;
		}

		static constexpr value_type neutral_value() noexcept
		{
			return 0.0f;
		}
	};

	template<>
	struct angle_traits<Degree> : base_angle_traits
	{
		static constexpr value_type to_neutral(value_type value) noexcept
		{
			return value * (std::numbers::pi_v<value_type> / 180.0f);
		}

		static constexpr value_type from_neutral(value_type value) noexcept
		{
			return value * (180.0f / std::numbers::pi_v<value_type>);
		}

		static constexpr auto name() noexcept
		{
			return "Degree"sv;
		}

		static constexpr value_type neutral_value() noexcept
		{
			return 0.0f;
		}
	};

	template<class Dest, class Src>
	constexpr typename angle_traits<Dest>::value_type
		angle_cast(const typename angle_traits<Src>::value_type&) noexcept;

	template<class T, class V = typename angle_traits<T>::value_type>
	class Angle
	{
	public:
		using value_type = V;

		Angle() = default;

		constexpr Angle(value_type value): _value(value)
		{
		}

		constexpr void swap(Angle& other) noexcept
		{
			std::swap(_value, other._value);
		}

		value_type value() const noexcept
		{
			return _value;
		}

		template<class U>
		constexpr Angle(const Angle<U>& other) noexcept
			: _value{ angle_cast<T, U>(other.value()) }
		{
		}

		template<class U>
		constexpr Angle& operator=(const Angle<U>& other) noexcept
		{
			Angle{ other }.swap(*this);
			return *this;
		}

		constexpr bool operator==(const Angle& other) const noexcept
		{
			return Approximate::ApproximatelyEqual(value(), other.value());
		}

		constexpr bool operator!=(const Angle& other) const noexcept
		{
			return !(*this == other);
		}

		constexpr bool operator<(const Angle& other) const noexcept
		{
			return value() < other.value();
		}

		constexpr bool operator<=(const Angle& other) const noexcept
		{
			return !(other.valeur() < value());
		}

		constexpr bool operator>(const Angle& other) const noexcept
		{
			return other.valeur() < value();
		}

		constexpr bool operator>=(const Angle& other) const noexcept
		{
			return !(value() < other.value());
		}

		constexpr Angle operator+(const Angle& other) const noexcept
		{
			return { value() + other.value() };
		}

		constexpr Angle operator-(const Angle& other) const noexcept
		{
			return { value() - other.value() };
		}

		constexpr Angle operator*(value_type scalar) const noexcept
		{
			return { value() * scalar };
		}

		constexpr Angle operator/(value_type scalar) const noexcept
		{
			return { value() / scalar };
		}

		constexpr Angle& operator+=(const Angle& other) noexcept
		{
			_value += other.value();
			return *this;
		}

		constexpr Angle& operator-=(const Angle& other) noexcept
		{
			_value -= other.value();
			return *this;
		}

		constexpr Angle& operator*=(value_type scalar) noexcept
		{
			_value *= scalar;
			return *this;
		}

		constexpr Angle& operator/=(value_type scalar) noexcept
		{
			_value /= scalar;
			return *this;
		}

		constexpr Angle& operator-() const noexcept
		{
			return { -value() };
		}

	private:
		value_type _value = angle_traits<T>::neutral_value();
	};


	template<class Dest, class Src>
	 constexpr typename angle_traits<Dest>::value_type
		 angle_cast(const typename angle_traits<Src>::value_type& src) noexcept
	{
		 return angle_traits<Dest>::from_neutral(
			 angle_traits<Src>::to_neutral(src)
		 );
	}

	 template<class T>
	 std::ostream& operator<<(std::ostream& os, const Angle<T>& angle)
	 {
		 os << angle._value << " " << angle_traits<T>::name();
		 return os;
	 }

	 constexpr Angle<Degree> operator"" _deg(long double value) noexcept
	 {
		 return { static_cast<float>(value) };
	 }

	 constexpr Angle<Degree> operator"" _deg(unsigned long long value) noexcept
	 {
		 return { static_cast<float>(value) };
	 }

	 constexpr Angle<Radian> operator"" _rad(long double value) noexcept
	 {
		 return { static_cast<float>(value) };
	 }

	 constexpr Angle<Radian> operator"" _rad(unsigned long long value) noexcept
	 {
		 return { static_cast<float>(value) };
	 }
}
