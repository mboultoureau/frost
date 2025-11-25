set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# assimp
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Build assimp samples" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build assimp tools" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Install assimp" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "Inject debug postfix" FORCE)

add_subdirectory(vendor/assimp)

if(WIN32 AND TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    target_compile_options(zlibstatic PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
    )
endif()

set_target_properties(
    UpdateAssimpLibsDebugSymbolsAndDLLs
    zlibstatic
    PROPERTIES FOLDER "Hidden Targets"
)
set_property(TARGET assimp PROPERTY FOLDER "Dependencies")

if(WIN32)
    set_target_properties(assimp PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(assimp PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
    )
endif()

set(ENABLE_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "Build Unit Tests" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "Build Hello World" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "Build Performance Test" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "Build Samples" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "Build JoltViewer" FORCE)

add_subdirectory(vendor/JoltPhysics/Build)
set_property(TARGET Jolt PROPERTY FOLDER "Dependencies")

if(WIN32)
    set_target_properties(Jolt PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(Jolt PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>

        # disable the enum warning for Jolt
        $<$<CXX_COMPILER_ID:MSVC>:/wd4865>
    )
endif()

# imgui
set(IMGUI_DIR "vendor/imgui")

set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)

add_library(ImGui STATIC
    ${IMGUI_SOURCES}
)

set_property(TARGET ImGui PROPERTY FOLDER "Dependencies")

target_include_directories(ImGui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

set_target_properties(ImGui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/ImGui"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/ImGui"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/ImGui"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/ImGui"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

if(WIN32)
    target_compile_definitions(ImGui PRIVATE
        IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    )
    
    target_compile_options(ImGui PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
    )

    target_link_libraries(ImGui PRIVATE
        d3d11.lib
    )
endif()

# spdlog
set(SPDLOG_DIR "vendor/spdlog")

set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build shared library" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use external fmt library instead of bundled" FORCE)
set(SPDLOG_FMT_EXTERNAL_HO OFF CACHE BOOL "Use external fmt header-only library instead of bundled" FORCE)
set(SPDLOG_NO_EXCEPTIONS OFF CACHE BOOL "Compile with -fno-exceptions" FORCE)

add_subdirectory("${SPDLOG_DIR}")

set_property(TARGET spdlog PROPERTY FOLDER "Dependencies")

if(WIN32)
    set_target_properties(spdlog PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(spdlog PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
    )
endif()

# stb
add_library(stb INTERFACE)
set_property(TARGET stb PROPERTY FOLDER "Dependencies")

target_include_directories(stb INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb"
)

# yaml-cpp
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tools" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable yaml-cpp contrib" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "Disable yaml-cpp install" FORCE)

add_subdirectory(vendor/yaml-cpp)

set_property(TARGET yaml-cpp PROPERTY FOLDER "Dependencies")

if(WIN32)
    set_target_properties(yaml-cpp PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(yaml-cpp PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
        
        # Suppression de certains warnings courants avec yaml-cpp sous MSVC
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4275>
    )
endif()

# entt
set(ENTT_BUILD_TESTING OFF CACHE BOOL "Build EnTT tests" FORCE)
set(ENTT_BUILD_DOCS OFF CACHE BOOL "Build EnTT documentation" FORCE)
set(ENTT_BUILD_TOOLS OFF CACHE BOOL "Build EnTT tools" FORCE)
set(ENTT_BUILD_EXAMPLES OFF CACHE BOOL "Build EnTT examples" FORCE)
set(ENTT_INSTALL OFF CACHE BOOL "Install EnTT" FORCE)

add_subdirectory(vendor/entt)

if(TARGET entt)
    set_property(TARGET entt PROPERTY FOLDER "Dependencies")
endif()

# Frost
file(GLOB_RECURSE FROST_SOURCES
    "src/**.h"
    "src/**.cpp"
)

add_library(Frost STATIC
    ${FROST_SOURCES}
)

set_property(TARGET Frost PROPERTY FOLDER "Engine")

set_target_properties(Frost PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Frost"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Frost"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Frost"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Frost"

    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INT_BASE_DIR}/$<CONFIG>-${CMAKE_SYSTEM_NAME}-x64/Frost"
)

target_include_directories(Frost PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
	${SPDLOG_DIR}/include
)

target_link_libraries(Frost PRIVATE
    d3d11.lib
    d3dcompiler.lib
    Xinput.lib
    assimp
    Jolt
    ImGui
    stb
    yaml-cpp
)

target_link_libraries(Frost PUBLIC
    spdlog::spdlog
    EnTT
)

if(WIN32)
    set_target_properties(Frost PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    target_compile_options(Frost PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
		$<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/bigobj>>
    )

    target_compile_definitions(Frost PUBLIC
        FT_PLATFORM_WINDOWS
        FT_BUILD_DLL
        UNICODE 
        _UNICODE
    )
endif()

target_compile_definitions(Frost PRIVATE
    $<$<CONFIG:Debug>:FT_DEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
)

target_compile_definitions(Frost PRIVATE
    $<$<CONFIG:Release>:FT_RELEASE>
)

target_compile_definitions(Frost PRIVATE
    $<$<CONFIG:Dist>:FT_DIST>
)

set_property(TARGET Frost APPEND PROPERTY COMPILE_OPTIONS
    $<$<CONFIG:Dist>:$<$<CXX_COMPILER_ID:MSVC>:/O2>>
)