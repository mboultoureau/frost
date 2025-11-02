# Jolt 3 project
file(GLOB_RECURSE JOLT3_SOURCES
    "src/**.h"
    "src/**.cpp"
)

add_executable(Jolt3 WIN32
    ${JOLT3_SOURCES}
)

set_property(TARGET Jolt3 PROPERTY FOLDER "Game")

set_target_properties(Jolt3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Jolt3"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Jolt3"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Jolt3"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Jolt3"

    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Jolt3"
)

target_include_directories(Jolt3 PRIVATE
    ${CMAKE_SOURCE_DIR}/Frost/src
)

target_link_libraries(Jolt3 PRIVATE
    Frost
	Jolt
)

if(WIN32)
    target_compile_options(Jolt3 PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )
    target_compile_definitions(Jolt3 PUBLIC
        FT_PLATFORM_WINDOWS
        UNICODE
        _UNICODE
    )
endif()

target_compile_definitions(Jolt3 PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
)

target_compile_definitions(Jolt3 PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(Jolt3 PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET Jolt3 APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)