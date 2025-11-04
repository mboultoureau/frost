file(GLOB_RECURSE LAB_SOURCES
    "src/**.h"
    "src/**.cpp"
)

add_executable(Lab WIN32
    ${LAB_SOURCES}
)

set_property(TARGET Lab PROPERTY FOLDER "Game")

set_target_properties(Lab PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Lab"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Lab"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Lab"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Lab"

    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Lab"
)

target_include_directories(Lab PRIVATE
    ${CMAKE_SOURCE_DIR}/Frost/src
)

target_link_libraries(Lab PRIVATE
    Frost
    Jolt
)

if(WIN32)
    target_compile_options(Lab PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
		$<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )
    target_compile_definitions(Lab PUBLIC
        FT_PLATFORM_WINDOWS
        UNICODE
        _UNICODE
    )
endif()

target_compile_definitions(Lab PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
)

target_compile_definitions(Lab PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(Lab PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET Lab APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)