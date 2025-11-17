# Game project
file(GLOB_RECURSE GAME_SOURCES
    "src/**.h"
    "src/**.cpp"
)

add_executable(Game WIN32
    ${GAME_SOURCES}
)

set_property(TARGET Game PROPERTY FOLDER "Game")

set_target_properties(Game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Game"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Game"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Game"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Game"

    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Game"
)

target_include_directories(Game PRIVATE
    ${CMAKE_SOURCE_DIR}/Frost/src
)

target_link_libraries(Game PRIVATE
    Frost
    Jolt
)

if(WIN32)
    target_compile_options(Game PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/bigobj>>
    )
    target_compile_definitions(Game PUBLIC
        FT_PLATFORM_WINDOWS
        UNICODE
        _UNICODE
    )
endif()

target_compile_definitions(Game PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
)

target_compile_definitions(Game PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(Game PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET Game APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)