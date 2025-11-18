cmake_minimum_required(VERSION 3.20)

project(Frost CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "Game")

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
add_subdirectory(Lab)
add_subdirectory(Game)