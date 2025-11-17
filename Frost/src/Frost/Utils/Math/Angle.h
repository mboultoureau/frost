#pragma once

#include <string_view>
#include <numbers>

#include "Frost/Utils/Math/Approximate.h"
#include "Frost/Utils/Math/Vector.h"

/**
* To learn more about changing the referential system, visit:
* @see https://h-deb.ca/Sujets/Divers--cplusplus/Implementer-changement-referentiel.html
*/

namespace Frost::Math
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
			return !(other.value() < value());
		}

		constexpr bool operator>(const Angle& other) const noexcept
		{
			return other.value() < value();
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

		constexpr Angle operator-() const noexcept
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

	 struct EulerAngles
	 {
		 Angle<Radian> Roll;
		 Angle<Radian> Yaw;
		 Angle<Radian> Pitch;
	 };

	 inline EulerAngles QuaternionToEulerAngles(const Vector<Vector4>& q)
	 {
		 EulerAngles angles;

		 const float x = q.value().x;
		 const float y = q.value().y;
		 const float z = q.value().z;
		 const float w = q.value().w;

		 // Roll (x-axis rotation)
		 float sinr_cosp = 2.0f * (w * x + y * z);
		 float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
		 angles.Roll = std::atan2(sinr_cosp, cosr_cosp);

		 // Pitch (y-axis rotation)
		 float sinp = 2.0f * (w * y - z * x);
		 if (std::abs(sinp) >= 1)
			 angles.Pitch = std::copysign(std::numbers::pi_v<float> / 2.0f, sinp);
		 else
			 angles.Pitch = std::asin(sinp);

		 // Yaw (z-axis rotation)
		 float siny_cosp = 2.0f * (w * z + x * y);
		 float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
		 angles.Yaw = std::atan2(siny_cosp, cosy_cosp);

		 return angles;
	 }

	 inline Vector4 EulerToQuaternion(const EulerAngles& angles)
	 {
		 // Yaw (Z), Pitch (Y), Roll (X)
		 float cy = std::cos(angles.Yaw.value() * 0.5f);
		 float sy = std::sin(angles.Yaw.value() * 0.5f);
		 float cp = std::cos(angles.Pitch.value() * 0.5f);
		 float sp = std::sin(angles.Pitch.value() * 0.5f);
		 float cr = std::cos(angles.Roll.value() * 0.5f);
		 float sr = std::sin(angles.Roll.value() * 0.5f);

		 Vector4 q;
		 q.w = cr * cp * cy + sr * sp * sy;
		 q.x = sr * cp * cy - cr * sp * sy;
		 q.y = cr * sp * cy + sr * cp * sy;
		 q.z = cr * cp * sy - sr * sp * cy;

		 return Vector4(q);
	 }
}
