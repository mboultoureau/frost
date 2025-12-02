#include "Frost/Utils/File/MemoryMappedFile.h"

namespace Frost
{
    MemoryMappedFile::MemoryMappedFile(const std::string& path)
    {
#ifdef FT_PLATFORM_WINDOWS
        _fileHandle = CreateFileA(path.c_str(),
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                  NULL);

        if (_fileHandle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        LARGE_INTEGER size;
        GetFileSizeEx(_fileHandle, &size);
        _size = static_cast<size_t>(size.QuadPart);

        _mapHandle = CreateFileMappingA(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        if (_mapHandle)
        {
            _data = MapViewOfFile(_mapHandle, FILE_MAP_READ, 0, 0, 0);
        }
#endif
    }

    MemoryMappedFile::~MemoryMappedFile()
    {

#ifdef FT_PLATFORM_WINDOWS
        if (_data)
        {
            UnmapViewOfFile(_data);
        }

        if (_mapHandle)
        {
            CloseHandle(_mapHandle);
        }

        if (_fileHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_fileHandle);
        }
#endif
    }

} // namespace Frost