@echo off
setlocal

set VCPKG_ROOT_DIR=vendor\vcpkg
set VCPKG_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
set VCPKG_INSTALL_DIR=vendor\installed
set BUILD_DIR=build

echo Starting automated project setup...

echo ---------------------------------------
echo 1. Updating/Initializing Git Submodules
echo ---------------------------------------

git submodule update --recursive

if errorlevel 1 (
    echo.
    echo [ERROR] Submodule update failed.
    echo Ensure Git is correctly installed and your .gitmodules file is configured.
    goto :end
)

echo Submodules updated successfully.
echo.

echo ---------------------------------------
echo 2. Setting Up Local Vcpkg in 'vendor'
echo ---------------------------------------

if exist "%VCPKG_ROOT_DIR%\vcpkg.exe" (
    echo Vcpkg is already bootstrapped. Skipping clone/bootstrap.
) else (
    if not exist "%VCPKG_ROOT_DIR%" (
        echo Cloning vcpkg into %VCPKG_ROOT_DIR%...
        git clone --depth 1 https://github.com/microsoft/vcpkg.git "%VCPKG_ROOT_DIR%"
        if errorlevel 1 (
            echo [ERROR] Vcpkg clone failed. Check Git and network connection.
            goto :end
        )
    )

    echo Bootstrapping vcpkg...
    call "%VCPKG_ROOT_DIR%\bootstrap-vcpkg.bat"
    if errorlevel 1 (
        echo [ERROR] VCPKG bootstrapping failed.
        goto :end
    )
)
echo Vcpkg setup complete.
echo.

echo ---------------------------------------
echo 3. Running CMake Configuration (in root directory)
echo ---------------------------------------

REM *** WARNING: CMake configuration is being run in the source root (-B .) as requested. ***
REM *** This will place the solution file (.sln) and ALL CMake cache files directly in the current directory. ***

echo Configuring project in the current directory (.\)...
REM The key change: -B . makes the current directory the binary directory.
REM Added -DVCPKG_INSTALLED_DIR to place compiled packages inside the vendor directory.

REM Retain 'call' and ensure path variables are quoted
call cmake -S . -B . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN_FILE%" -DVCPKG_INSTALLED_DIR="%VCPKG_INSTALL_DIR%" -G "Visual Studio 17 2022"

REM Check the errorlevel immediately after the call
if errorlevel 1 goto :cmake_error

goto :cmake_success

:cmake_error
echo.
echo [ERROR] CMake configuration failed.
echo Ensure CMake and a compatible C++ compiler (like Visual Studio) are installed.
goto :end

:cmake_success
echo =======================================
echo BUILD SETUP COMPLETE.
echo The Solution file (.sln) is now in the root directory.
echo Vcpkg installed files are now isolated in: %VCPKG_INSTALL_DIR%
echo =======================================

:end
pause
endlocal