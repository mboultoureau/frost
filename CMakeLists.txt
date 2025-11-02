cmake_minimum_required(VERSION 3.20)

project(Frost CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "Lab")

set(CMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "Available configurations" FORCE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_BASE_DIR "bin")
set(INT_BASE_DIR "bin-int")

add_subdirectory(Frost)
add_subdirectory(Lab)
add_subdirectory(Jolt3)