#pragma once

#include <Jolt/Jolt.h>
#include <string_view>
#include <numbers>
#include <algorithm>
#include <DirectXMath.h>
#include <ostream>
#include <type_traits>

namespace Frost::Math
{
    using VectorValueType = float;

    struct Vector2 {
        union
        {
            struct { float u, v; };
            struct { float x, y; };
            float values[2];
        };

        constexpr Vector2() noexcept : x(0.0f), y(0.0f) {}
        constexpr Vector2(float x, float y) noexcept : x(x), y(y) {}
    };

    struct Vector3 {
        union
        {
            struct { float r, g, b; };
            struct { float x, y, z; };
			struct { float width, height, depth; };
            float values[3];
        };

        constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
    };


    struct Vector4 {
        union
        {
            struct { float r, g, b, a; };
            struct { float x, y, z, w; };
            float values[4];
        };

        constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        constexpr Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
    };

    using Color3 = Vector3;
	using Color4 = Vector4;

    struct base_vector_traits
    {
        using value_type = VectorValueType;
        using neutral_type = Vector4;
    };

    template<class T>
    struct vector_traits;

    template<>
    struct vector_traits<Vector4> : base_vector_traits
    {
        using internal_type = Vector4;
        static constexpr int Dimensions = 4;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
            return value;
        }

        static constexpr internal_type from_neutral(const internal_type& value) noexcept
        {
            return value;
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "Vector4"sv;
        }

        static constexpr internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<Vector3> : base_vector_traits
    {
        using internal_type = Vector3;
        static constexpr int Dimensions = 3;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.x, value.y, value.z, 0.0f };
        }

        static constexpr internal_type from_neutral(const neutral_type& value) noexcept
        {
            return internal_type{ value.x, value.y, value.z };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "Vector3"sv;
        }

        static constexpr internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<DirectX::XMFLOAT3> : base_vector_traits
    {
        using internal_type = DirectX::XMFLOAT3;
        static constexpr int Dimensions = 3;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.x, value.y, value.z, 0.0f };
        }

        static constexpr internal_type from_neutral(const neutral_type& value) noexcept
        {
            return internal_type{ value.x, value.y, value.z };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "XMFLOAT3"sv;
        }

        static constexpr internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<DirectX::XMFLOAT4> : base_vector_traits
    {
        using internal_type = DirectX::XMFLOAT4;
        static constexpr int Dimensions = 4;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.x, value.y, value.z, value.w };
        }

        static constexpr internal_type from_neutral(const neutral_type& value) noexcept
        {
            return internal_type{ value.x, value.y, value.z, value.w };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "XMFLOAT4"sv;
        }

        static constexpr internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<DirectX::XMVECTOR> : base_vector_traits
    {
        using internal_type = DirectX::XMVECTOR;
        static constexpr int Dimensions = 4;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
			return neutral_type{ DirectX::XMVectorGetX(value), DirectX::XMVectorGetY(value), DirectX::XMVectorGetZ(value), DirectX::XMVectorGetW(value) };
        }

        static constexpr internal_type from_neutral(const neutral_type& value) noexcept
        {
            return internal_type{ value.x, value.y, value.z, value.w };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "XMVECTOR"sv;
        }

        static constexpr internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<JPH::Vec3> : base_vector_traits
    {
        using internal_type = JPH::Vec3;
        static constexpr int Dimensions = 3;

        static neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.GetX(), value.GetY(), value.GetZ(), 0.0f};
        }

        static internal_type from_neutral(const neutral_type& value) noexcept
        {
			return internal_type{ value.x, value.y, value.z };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "JPH::Vec3"sv;
        }

        static internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<JPH::Vec4> : base_vector_traits
    {
        using internal_type = JPH::Vec4;
        static constexpr int Dimensions = 4;

        static neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.GetX(), value.GetY(), value.GetZ(), value.GetW() };
        }

        static internal_type from_neutral(const neutral_type& value) noexcept
        {
            return internal_type{ value.x, value.y, value.z, value.w };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "JPH::Vec4"sv;
        }

        static internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f };
        }
    };

    template<>
    struct vector_traits<Vector2> : base_vector_traits
    {
        using internal_type = Vector2;
        static constexpr int Dimensions = 2;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.x, value.y, 0.0f, 0.0f };
        }

        static constexpr internal_type from_neutral(const neutral_type& value) noexcept
        {
            return internal_type{ value.x, value.y };
        }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "Vector2"sv;
        }

        static constexpr internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f };
        }
    };

    template<class Dest, class Src>
    constexpr auto vector_cast(Src&& src) noexcept
    {
        using SrcType = std::remove_cvref_t<Src>;
        return vector_traits<Dest>::from_neutral(
            vector_traits<SrcType>::to_neutral(std::forward<Src>(src))
        );
    }

    template<class T>
    class Vector
    {
    public:
        using traits = vector_traits<T>;
		using neutral_type = typename traits::neutral_type;
        using internal_type = typename traits::internal_type;
        using value_type = typename traits::value_type;

        Vector() = default;

        constexpr Vector(internal_type value) : _value(value)
        {
        }

        constexpr void swap(Vector& other) noexcept
        {
            std::swap(_value, other._value);
        }

        internal_type value() const noexcept
        {
            return _value;
        }

        template<class U>
        constexpr explicit Vector(const Vector<U>& other) noexcept
            : _value{ vector_cast<T, U>(other.value()) }
        {
        }

        template<class U>
        constexpr Vector& operator=(const Vector<U>& other) noexcept
        {
            Vector{ other }.swap(*this);
            return *this;
        }

        constexpr Vector operator+(const Vector& other) const noexcept
        {
            auto a_neutral = traits::to_neutral(_value);
            auto b_neutral = traits::to_neutral(other._value);

            neutral_type result_neutral;
            for (int i = 0; i < traits::Dimensions; ++i)
            {
                result_neutral.values[i] = a_neutral.values[i] + b_neutral.values[i];
            }

            return Vector{ traits::from_neutral(result_neutral) };
        }

        constexpr Vector operator-(const Vector& other) const noexcept
        {
            auto a_neutral = traits::to_neutral(_value);
            auto b_neutral = traits::to_neutral(other._value);
            neutral_type result_neutral;
            for (int i = 0; i < traits::Dimensions; ++i)
            {
                result_neutral.values[i] = a_neutral.values[i] - b_neutral.values[i];
            }
            return Vector{ traits::from_neutral(result_neutral) };
		}

        constexpr Vector& operator+=(const Vector& other) noexcept
        {
            *this = *this + other;
            return *this;
		}

        constexpr Vector& operator-=(const Vector& other) noexcept
        {
            *this = *this - other;
            return *this;
        }

        constexpr Vector operator-() const noexcept
        {
            auto neutral = traits::to_neutral(_value);
            neutral_type result_neutral;
            for (int i = 0; i < traits::Dimensions; ++i)
            {
                result_neutral.values[i] = -neutral.values[i];
            }
            return Vector{ traits::from_neutral(result_neutral) };
		}

        constexpr Vector& operator=(const internal_type& value) noexcept
        {
            _value = value;
            return *this;
		}

        constexpr Vector& operator=(internal_type&& value) noexcept
        {
            _value = std::move(value);
            return *this;
		}

        constexpr Vector(internal_type&& value) noexcept : _value(std::move(value))
        {
		}

    private:
        internal_type _value = traits::neutral_value();
    };

    template<class T>
    std::ostream& operator<<(std::ostream& os, const Vector<T>& vector)
    {
        os << vector_traits<T>::name() << "(";
        auto neutral = vector_traits<T>::to_neutral(vector.value());
        for (int i = 0; i < vector_traits<T>::Dimensions; ++i)
        {
            os << neutral.values[i];
            if (i < vector_traits<T>::Dimensions - 1)
            {
                os << ", ";
            }
        }
        os << ")";
		return os;
    }
}