workspace "Frost"
    architecture "x64"
    startproject "Lab"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Frost"
    location "Frost"
    kind "StaticLib"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/%{prj.name}")
    objdir ("bin-int/" .. output_dir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Frost/src"
    }

    links
    {
        "d3d11.lib"
    }

    filter "system:windows"
        cppdialect "C++23"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "FT_PLATFORM_WINDOWS",
            "FT_BUILD_DLL"
        }

    filter "configurations:Debug"
        defines "FT_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "FT_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "FT_DIST"
        symbols "On"

project "Lab"
    location "Lab"
    kind "WindowedApp"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/%{prj.name}")
    objdir ("bin-int/" .. output_dir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Frost/src"
    }

    links
    {
        "Frost"
    }

    filter "system:windows"
        cppdialect "C++23"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "FT_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "FT_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "FT_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "FT_DIST"
        symbols "On"