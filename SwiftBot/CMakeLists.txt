# Game
file(GLOB_RECURSE SWIFT_BOT_SOURCES
    "src/**.h"
    "src/**.cpp"
)

add_executable(SwiftBot WIN32
    ${SWIFT_BOT_SOURCES}
)

set_property(TARGET SwiftBot PROPERTY FOLDER "Game")

set_target_properties(SwiftBot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBot"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBot"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBot"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBot"

    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBot"
)

target_include_directories(SwiftBot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/Frost/src
)

target_compile_definitions(SwiftBot PRIVATE JPH_SHARED_LIBRARY)

target_link_libraries(SwiftBot PRIVATE
    Frost
    ImGui
    Jolt
)

if(WIN32)
    set_target_properties(SwiftBot PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    target_compile_options(SwiftBot PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MDd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MD>>
        
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/bigobj>>
    )
    target_compile_definitions(SwiftBot PUBLIC
        FT_PLATFORM_WINDOWS
        UNICODE
        _UNICODE
    )
endif()

target_compile_definitions(SwiftBot PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
)

target_compile_definitions(SwiftBot PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(SwiftBot PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET SwiftBot APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)

add_custom_command(TARGET SwiftBot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:Frost>
    $<TARGET_FILE_DIR:SwiftBot>
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Jolt>
    $<TARGET_FILE_DIR:SwiftBot>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:assimp>
    $<TARGET_FILE_DIR:SwiftBot>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:spdlog>
    $<TARGET_FILE_DIR:SwiftBot>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:yaml-cpp>
    $<TARGET_FILE_DIR:SwiftBot>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:assimp>
    $<TARGET_FILE_DIR:SwiftBot>
)

# Game Logic
file(GLOB_RECURSE SWIFT_BOT_LOGIC_SOURCES
    "scripts/**.h"
    "scripts/**.cpp"
)

add_library(SwiftBotLogic SHARED
    ${SWIFT_BOT_LOGIC_SOURCES}
)

set_property(TARGET SwiftBotLogic PROPERTY FOLDER "Game")

set_target_properties(SwiftBotLogic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBotLogic"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBotLogic"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBotLogic"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBotLogic"
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/SwiftBotLogic"
)

target_compile_definitions(SwiftBotLogic PRIVATE JPH_SHARED_LIBRARY)

target_include_directories(SwiftBotLogic PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts
    ${CMAKE_SOURCE_DIR}/Frost/src
)

target_link_libraries(SwiftBotLogic PRIVATE
    Frost
    ImGui
    Jolt
)

if(WIN32)
    target_compile_options(SwiftBotLogic PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MDd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MD>>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/bigobj>>
    )
    target_compile_definitions(SwiftBotLogic PUBLIC
        FT_PLATFORM_WINDOWS
        UNICODE
        _UNICODE
    )
endif()

target_compile_definitions(SwiftBotLogic PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
)

target_compile_definitions(SwiftBotLogic PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(SwiftBotLogic PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET SwiftBotLogic APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)

add_custom_command(TARGET SwiftBotLogic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SwiftBotLogic>
    $<TARGET_FILE_DIR:SwiftBot>
)