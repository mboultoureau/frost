@echo off

echo Starting project setup...

echo ---------------------------------------
echo 1. Updating/Initializing Git Submodules
echo ---------------------------------------

git submodule update --init --recursive

if errorlevel 1 (
    echo.
    echo [ERROR] Submodule update failed.
    echo Ensure Git is correctly installed and your .gitmodules file is configured.
    goto :end
)

echo Submodules updated successfully.
echo.

echo ---------------------------------------
echo 2. Running CMake Configuration (cmake .)
echo ---------------------------------------

cmake .

if errorlevel 1 (
    echo.
    echo [ERROR] CMake configuration failed.
    echo Ensure CMake is correctly installed and in your system PATH.
    goto :end
)

echo.
echo =======================================
echo BUILD SETUP COMPLETE.
echo You can now proceed with building your project.
echo =======================================

:end
pause
