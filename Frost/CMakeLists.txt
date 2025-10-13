set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

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
)

target_link_libraries(Frost PRIVATE
    d3d11.lib
    d3dcompiler.lib
    assimp
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