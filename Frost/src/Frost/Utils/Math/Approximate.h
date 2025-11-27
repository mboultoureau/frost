#pragma once

#include <concepts>
#include <utility>

/**
 * @see https://h-deb.ca/Sujets/Maths/Assez-proches.html
 */

namespace Frost::Math
{
    class Approximate
    {
        // Check if two values are approximately equal
        template<class T>
        static constexpr T epsilon = static_cast<T>(1e-5);

        constexpr bool ApproximatelyEqual(std::integral auto a, std::integral auto b) const noexcept
        {
            return std::cmp_equal(a, b);
        }

        template<class T>
        constexpr T abs(T value) const noexcept
        {
            return (value < 0) ? -value : value;
        }

        constexpr bool ApproximatelyEqual(std::floating_point auto a, std::floating_point auto b) const noexcept
        {
            return abs(a - b) <= epsilon<std::common_type_t<decltype(a), decltype(b)>>;
        }
    };
} // namespace Frost::Math
