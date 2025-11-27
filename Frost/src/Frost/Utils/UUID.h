#pragma once

#include <cstdint>

namespace Frost
{
    /**
     * @see https://skypjack.github.io/2019-02-14-ecs-baf-part-1/
     */
    template<typename T>
    class UUID
    {
    public:
        using ValueType = uint64_t;

        UUID() noexcept : _id{ generate().value() } {}
        explicit UUID(ValueType id) noexcept : _id{ id } {}

        static UUID<T> generate() noexcept { return UUID<T>(nextId()); }

        ValueType value() const noexcept { return _id; }

        bool operator==(const UUID<T>& other) const noexcept { return _id == other._id; }

        bool operator!=(const UUID<T>& other) const noexcept { return !(*this == other); }

        struct Hash
        {
            std::size_t operator()(const UUID<T>& uuid) const noexcept { return std::hash<ValueType>()(uuid.value()); }
        };

    private:
        ValueType _id;

        static ValueType nextId() noexcept
        {
            static ValueType _nextId = 0;
            return ++_nextId;
        }
    };
} // namespace Frost
