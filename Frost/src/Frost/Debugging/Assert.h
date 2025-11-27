#pragma once

#include "Frost/Debugging/Logger.h"

namespace Frost
{
#ifdef FT_PLATFORM_WINDOWS
#define FT_DEBUG_BREAK() __debugbreak()
#else
#define FT_DEBUG_BREAK() (void)0
#endif

#define FT_ASSERT_INTERNAL_NO_MSG(log_func, condition)                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            log_func("Assertion failed: {} [{}:{}]", #condition, __FILE__, __LINE__);                                  \
            FT_DEBUG_BREAK();                                                                                          \
        }                                                                                                              \
    } while (0)

#define FT_ASSERT_INTERNAL_WITH_MSG(log_func, condition, custom_msg_format, ...)                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            log_func(                                                                                                  \
                "Assertion failed: {} [{}:{}] | " custom_msg_format, #condition, __FILE__, __LINE__, ##__VA_ARGS__);   \
            FT_DEBUG_BREAK();                                                                                          \
        }                                                                                                              \
    } while (0)

#define FT_EXPAND(x) x
#define FT_PICK_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME
#define FT_CHOOSE_ASSERT_MACRO(...)                                                                                    \
    FT_EXPAND(FT_PICK_MACRO(__VA_ARGS__,                                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_WITH_MSG,                                                               \
                            FT_ASSERT_INTERNAL_NO_MSG))

#define FT_ASSERT(...) FT_EXPAND(FT_CHOOSE_ASSERT_MACRO(__VA_ARGS__)(::Frost::FT_CRITICAL, __VA_ARGS__))

#define FT_ENGINE_ASSERT(...) FT_EXPAND(FT_CHOOSE_ASSERT_MACRO(__VA_ARGS__)(::Frost::FT_ENGINE_CRITICAL, __VA_ARGS__))
} // namespace Frost
