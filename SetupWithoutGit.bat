@echo off
setlocal EnableDelayedExpansion

:: Setup colors
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set "ESC=%%b"
)

set "RED=%ESC%[91m"
set "GREEN=%ESC%[92m"
set "YELLOW=%ESC%[93m"
set "RESET=%ESC%[0m"
set "BOLD=%ESC%[1m"

:: Setup
echo %BOLD%Initializing Project Environment...%RESET%
echo.

:: -----------------------------------------------------------
echo [STEP 1/4] Initializing Git Submodules
:: -----------------------------------------------------------

git init
git submodule add https://github.com/assimp/assimp.git .\Frost\vendor\assimp
git submodule add https://github.com/jrouwe/JoltPhysics.git .\Frost\vendor\JoltPhysics
git submodule add  -b docking https://github.com/ocornut/imgui.git .\Frost\vendor\imgui
git submodule add https://github.com/gabime/spdlog.git .\Frost\vendor\spdlog
git submodule add https://github.com/nothings/stb .\Frost\vendor\stb
git submodule add https://github.com/skypjack/entt.git .\Frost\vendor\entt
git submodule add https://github.com/jbeder/yaml-cpp.git .\Frost\vendor\yaml-cpp

if errorlevel 1 (
    echo.
    echo %RED%[ERROR] Submodule update failed.%RESET%
    echo Ensure Git is correctly installed and your .gitmodules file is configured.
    goto :end
)

echo Submodules updated successfully.
echo.

:: -----------------------------------------------------------
echo [STEP 2/4] Configuring Build System (CMake)
:: -----------------------------------------------------------

cmake .

if errorlevel 1 (
    echo.
    echo %RED%[ERROR] CMake configuration failed.%RESET%
    echo Ensure CMake is correctly installed and in your system PATH.
    goto :end
)

echo.
:: -----------------------------------------------------------
echo [STEP 3/4] Verifying Clang-Format Installation
:: -----------------------------------------------------------

where clang-format >nul 2>nul

if %errorlevel% neq 0 (
    echo.
    echo %YELLOW%[WARNING] 'clang-format' was NOT found in your PATH!%RESET%
    echo %YELLOW%Automatic code formatting will not work.%RESET%
    echo.
    echo ========================================================
    echo HOW TO FIX:
    echo 1. Via Visual Studio (If installed with C++^):
    echo    You likely already have it. Add this path to your Environment Variables:
    echo    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin"
    echo    (Check your specific VS version/edition path^).
    echo.
    echo 2. Via Winget (PowerShell^):
    echo    Run: winget install LLVM.LLVM
    echo.
    echo 3. Manual Download:
    echo    https://github.com/llvm/llvm-project/releases
    echo    (Select "Add LLVM to the system PATH" during install^).
    echo.
    echo NOTE: You must restart your terminal after changes.
    echo ========================================================
    echo.
) else (
    echo clang-format is installed and ready.
)

echo.
:: -----------------------------------------------------------
echo [STEP 4/4] Installing Git Hooks
:: -----------------------------------------------------------

git config core.hooksPath scripts

if errorlevel 1 (
    echo.
    echo %RED%[ERROR] Failed to configure Git hooks.%RESET%
    echo Ensure the 'scripts' directory exists at the project root.
) else (
    echo %GREEN%Git hooks configured successfully.%RESET%
    echo Pre-commit hook is now active (auto-formatting enabled^).
)

echo.
echo =======================================
echo %GREEN%BUILD SETUP COMPLETE.%RESET%
echo You can now proceed with building your project.
echo =======================================

:end
pause
