cmake_minimum_required(VERSION 3.20)

project(Frost CXX)

set(CMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "Available configurations" FORCE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_BASE_DIR "bin")
set(INT_BASE_DIR "bin-int")

# Compilation optimizations
if(MSVC)
    add_compile_options(/MP)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N} CACHE STRING "Number of parallel jobs to use")
    endif()
endif()

add_subdirectory(Frost)
add_subdirectory(Editor)
add_subdirectory(Lab)
add_subdirectory(Game)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "Editor")

find_program(CLANG_FORMAT_EXE clang-format)

if(NOT CLANG_FORMAT_EXE)
    find_program(CLANG_FORMAT_EXE clang-format)
endif()

if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE FILES_TO_FORMAT
        "${CMAKE_CURRENT_SOURCE_DIR}/Frost/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Frost/src/*.h"
        
        "${CMAKE_CURRENT_SOURCE_DIR}/Editor/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Editor/src/*.h"
        
        "${CMAKE_CURRENT_SOURCE_DIR}/Game/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Game/src/*.h"
        
        "${CMAKE_CURRENT_SOURCE_DIR}/Lab/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Lab/src/*.h"
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${FILES_TO_FORMAT}
        COMMENT "Auto-formatting sources..."
        VERBATIM
    )
    set_target_properties(format PROPERTIES FOLDER "Tools")

    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror -style=file ${FILES_TO_FORMAT}
        COMMENT "Checking code formatting..."
        VERBATIM
    )
    set_target_properties(check-format PROPERTIES FOLDER "Tools")

    message(STATUS "Clang-format found: target 'format' is available.")
else()
    message(WARNING "Clang-format not found. The 'format' target will not be created.")
endif()
