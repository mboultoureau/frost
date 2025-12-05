#pragma once

#include <DirectXMath.h>
#include <Jolt/Jolt.h>
#include <algorithm>
#include <numbers>
#include <ostream>
#include <string_view>
#include <type_traits>

namespace Frost::Math
{
    class Matrix4x4;

    using VectorValueType = float;

    struct Vector2
    {
        union
        {
            struct
            {
                float u, v;
            };
            struct
            {
                float x, y;
            };
            float values[2];
        };

        constexpr Vector2() noexcept : x(0.0f), y(0.0f) {}
        constexpr Vector2(float x, float y) noexcept : x(x), y(y) {}
        constexpr Vector2(const DirectX::XMFLOAT2& v) noexcept : x(v.x), y(v.y) {}
        operator DirectX::XMFLOAT2() const { return DirectX::XMFLOAT2(x, y); }
    };

    struct Vector3
    {
        union
        {
            struct
            {
                float r, g, b;
            };
            struct
            {
                float x, y, z;
            };
            struct
            {
                float width, height, depth;
            };
            float values[3];
        };

        constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
        constexpr Vector3(const DirectX::XMFLOAT3& v) noexcept : x(v.x), y(v.y), z(v.z) {}
        operator DirectX::XMFLOAT3() const { return DirectX::XMFLOAT3(x, y, z); }
    };

    struct Vector4
    {
        union
        {
            struct
            {
                float r, g, b, a;
            };
            struct
            {
                float x, y, z, w;
            };
            float values[4];
        };

        constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        constexpr Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
        constexpr Vector4(const DirectX::XMFLOAT4& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) {}
        operator DirectX::XMFLOAT4() const { return DirectX::XMFLOAT4(x, y, z, w); }
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

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept { return value; }

        static constexpr internal_type from_neutral(const internal_type& value) noexcept { return value; }

        static constexpr auto name() noexcept
        {
            using namespace std::string_view_literals;
            return "Vector4"sv;
        }

        static constexpr internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f }; }
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

        static constexpr internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f }; }
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

        static constexpr internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f }; }
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

        static constexpr internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f }; }
    };

    template<>
    struct vector_traits<DirectX::XMVECTOR> : base_vector_traits
    {
        using internal_type = DirectX::XMVECTOR;
        static constexpr int Dimensions = 4;

        static constexpr neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ DirectX::XMVectorGetX(value),
                                 DirectX::XMVectorGetY(value),
                                 DirectX::XMVectorGetZ(value),
                                 DirectX::XMVectorGetW(value) };
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

        static constexpr internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f, 0.0f }; }
    };

    template<>
    struct vector_traits<JPH::Vec3> : base_vector_traits
    {
        using internal_type = JPH::Vec3;
        static constexpr int Dimensions = 3;

        static neutral_type to_neutral(const internal_type& value) noexcept
        {
            return neutral_type{ value.GetX(), value.GetY(), value.GetZ(), 0.0f };
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

        static internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f }; }
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

        static internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f, 0.0f, 1.0f }; }
    };

    template<>
    struct vector_traits<JPH::Quat> : base_vector_traits
    {
        using internal_type = JPH::Quat;
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
            return "JPH::Quat"sv;
        }

        static internal_type neutral_value() noexcept
        {
            return internal_type{ 0.0f, 0.0f, 0.0f, 1.0f }; // quaternion identite
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

        static constexpr internal_type neutral_value() noexcept { return internal_type{ 0.0f, 0.0f }; }
    };

    template<class Dest, class Src>
    constexpr auto vector_cast(Src&& src) noexcept
    {
        using SrcType = std::remove_cvref_t<Src>;
        return vector_traits<Dest>::from_neutral(vector_traits<SrcType>::to_neutral(std::forward<Src>(src)));
    }

    template<typename T>
    concept IsVector = requires {
        typename vector_traits<T>::neutral_type;
        vector_traits<T>::Dimensions;
    };

    template<IsVector T>
    constexpr T operator-(const T& lhs, const T& rhs) noexcept
    {
        using traits = vector_traits<T>;

        auto a_neutral = traits::to_neutral(lhs);
        auto b_neutral = traits::to_neutral(rhs);

        typename traits::neutral_type result_neutral;
        for (int i = 0; i < traits::Dimensions; ++i)
        {
            result_neutral.values[i] = a_neutral.values[i] - b_neutral.values[i];
        }

        return traits::from_neutral(result_neutral);
    }

    template<IsVector T>
    constexpr T operator+(const T& lhs, const T& rhs) noexcept
    {
        using traits = vector_traits<T>;
        auto a_neutral = traits::to_neutral(lhs);
        auto b_neutral = traits::to_neutral(rhs);

        typename traits::neutral_type result_neutral;
        for (int i = 0; i < traits::Dimensions; ++i)
        {
            result_neutral.values[i] = a_neutral.values[i] + b_neutral.values[i];
        }

        return traits::from_neutral(result_neutral);
    }

    template<IsVector T>
    constexpr T operator*(const T& vec, float scalar) noexcept
    {
        using traits = vector_traits<T>;
        auto neutral = traits::to_neutral(vec);

        typename traits::neutral_type res;
        for (int i = 0; i < traits::Dimensions; ++i)
        {
            res.values[i] = neutral.values[i] * scalar;
        }

        return traits::from_neutral(res);
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

        constexpr Vector(internal_type value) : _value(value) {}

        constexpr void swap(Vector& other) noexcept { std::swap(_value, other._value); }

        internal_type value() const noexcept { return _value; }

        template<class U>
        constexpr explicit Vector(const Vector<U>& other) noexcept : _value{ vector_cast<T, U>(other.value()) }
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

        constexpr Vector(internal_type&& value) noexcept : _value(std::move(value)) {}

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

    inline float Dot(const Vector3& a, const Vector3& b)
    {
        DirectX::XMVECTOR vA = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&a);
        DirectX::XMVECTOR vB = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&b);
        return DirectX::XMVectorGetX(DirectX::XMVector3Dot(vA, vB));
    }

    inline Vector3 Cross(const Vector3& a, const Vector3& b)
    {
        DirectX::XMVECTOR vA = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&a);
        DirectX::XMVECTOR vB = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&b);
        Vector3 result;
        DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&result, DirectX::XMVector3Cross(vA, vB));
        return result;
    }

    inline Vector3 Normalize(const Vector3& v)
    {
        DirectX::XMVECTOR vec = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&v);
        Vector3 result;
        DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&result, DirectX::XMVector3Normalize(vec));
        return result;
    }

    inline float Length(const Vector3& v)
    {
        DirectX::XMVECTOR vec = DirectX::XMLoadFloat3((const DirectX::XMFLOAT3*)&v);
        return DirectX::XMVectorGetX(DirectX::XMVector3Length(vec));
    }

    /**
     * @see https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
     */
    inline Color3 KelvinToRGB(float kelvin)
    {
        float temp = kelvin / 100.0f;
        float red, green, blue;

        if (temp <= 66.0f)
        {
            red = 255.0f;
            green = temp;
            green = 99.4708025861f * std::log(green) - 161.1195681661f;

            if (temp <= 19.0f)
                blue = 0.0f;
            else
            {
                blue = temp - 10.0f;
                blue = 138.5177312231f * std::log(blue) - 305.0447927307f;
            }
        }
        else
        {
            red = temp - 60.0f;
            red = 329.698727446f * std::pow(red, -0.1332047592f);

            green = temp - 60.0f;
            green = 288.1221695283f * std::pow(green, -0.0755148492f);

            blue = 255.0f;
        }

        auto clamp = [](float v) { return v < 0.0f ? 0.0f : (v > 255.0f ? 255.0f : v); };
        return { clamp(red) / 255.0f, clamp(green) / 255.0f, clamp(blue) / 255.0f };
    };
} // namespace Frost::Math