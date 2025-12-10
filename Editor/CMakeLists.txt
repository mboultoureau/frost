# Editor
file(GLOB_RECURSE EDITOR_SOURCES
    "src/**.h"
    "src/**.cpp"
)

add_executable(Editor WIN32
    ${EDITOR_SOURCES}
)

set_property(TARGET Editor PROPERTY FOLDER "Engine")

set_target_properties(Editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Editor"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Editor"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Editor"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Editor"

    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Editor"
)

target_include_directories(Editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/Frost/src
)

target_link_libraries(Editor PRIVATE
    Frost
    ImGui
    Jolt
    yaml-cpp
)

if(WIN32)
    set_target_properties(Editor PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    target_compile_options(Editor PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MDd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MD>>

        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/bigobj>>
    )
    target_compile_definitions(Editor PUBLIC
        FT_PLATFORM_WINDOWS
        UNICODE
        _UNICODE
    )
endif()

target_compile_definitions(Editor PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
)

target_compile_definitions(Editor PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(Editor PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET Editor APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Frost>
    $<TARGET_FILE_DIR:Editor>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Jolt>
    $<TARGET_FILE_DIR:Editor>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:assimp>
    $<TARGET_FILE_DIR:Editor>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:spdlog>
    $<TARGET_FILE_DIR:Editor>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:yaml-cpp>
    $<TARGET_FILE_DIR:Editor>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:assimp>
    $<TARGET_FILE_DIR:Editor>
)