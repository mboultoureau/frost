set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# assimp
add_subdirectory(vendor/assimp)
set_target_properties(
    uninstall
    unit
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

# GameInput
add_library(Microsoft::GameInput STATIC IMPORTED)

set_target_properties(Microsoft::GameInput PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GameInput/include"
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GameInput/lib/x64/GameInput.lib"
    FOLDER "Dependencies"
)

# Jolt Physics
add_subdirectory(vendor/JoltPhysics/Build)
set_property(TARGET Jolt PROPERTY FOLDER "Dependencies")

if(WIN32)
    set_target_properties(Jolt PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(Jolt PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
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
    $<TARGET_PROPERTY:Microsoft::GameInput,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(Frost PRIVATE
    d3d11.lib
    d3dcompiler.lib
    assimp
    Jolt
    ImGui
    Microsoft::GameInput
)

if(WIN32)
    set_target_properties(Frost PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    target_compile_options(Frost PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/MTd>>
        $<$<NOT:$<CONFIG:Debug>>:$<$<CXX_COMPILER_ID:MSVC>:/MT>>
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