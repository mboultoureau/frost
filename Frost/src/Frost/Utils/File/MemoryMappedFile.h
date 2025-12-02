#pragma once
#include "Frost/Debugging/Assert.h"

#include <string>
#include <span>

#ifdef FT_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Frost
{
    class MemoryMappedFile
    {
    public:
        MemoryMappedFile(const std::string& path);
        ~MemoryMappedFile();

        bool IsValid() const { return _data != nullptr; }
        const void* GetData() const { return _data; }
        size_t GetSize() const { return _size; }
        std::span<const uint8_t> GetSpan() const { return { static_cast<const uint8_t*>(_data), _size }; }

    private:
        void* _data = nullptr;
        size_t _size = 0;
#ifdef FT_PLATFORM_WINDOWS
        HANDLE _fileHandle = INVALID_HANDLE_VALUE;
        HANDLE _mapHandle = NULL;
#endif
    };
} // namespace Frost