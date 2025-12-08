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
echo %BOLD%Initializing Project Environment (Clang step removed)...%RESET%
echo.

:: -----------------------------------------------------------
echo [STEP 1/3] Initializing Git Submodules
:: -----------------------------------------------------------

:: Initialisation d'un depot Git temporaire si non trouve, pour permettre le telechargement des dependances.
if not exist .git (
    echo %YELLOW%[NOTICE] .git folder not found. Initializing a temporary Git repository...%RESET%
    
    git init >nul 2>&1
    
    if errorlevel 1 (
        echo %RED%[ERROR] Git initialisation failed. Ensure Git is installed and in your PATH.%RESET%
        goto :end
    )
    
    if not exist .gitmodules (
        echo %RED%[FATAL ERROR] Cannot find .gitmodules file!%RESET%
        echo The script needs the .gitmodules file to know which dependencies to download.
        goto :end
    )
    
    echo %GREEN%Temporary Git repository created.%RESET%
)

git submodule update --init --recursive --depth 1

if errorlevel 1 (
    echo.
    echo %RED%[ERROR] Submodule update failed.%RESET%
    echo Ensure Git is correctly installed and your .gitmodules file is configured.
    goto :end
)

echo Submodules updated successfully.
echo.

:: -----------------------------------------------------------
echo [STEP 2/3] Configuring Build System (CMake)
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
echo [STEP 3/3] Installing Git Hooks
:: -----------------------------------------------------------

git config core.hooksPath scripts

if errorlevel 1 (
    echo.
    echo %RED%[ERROR] Failed to configure Git hooks.%RESET%
    echo Ensure the 'scripts' directory exists at the project root.
) else (
    echo %GREEN%Git hooks configured successfully.%RESET%
    echo Pre-commit hook is now active.
)

echo.
echo =======================================
echo %GREEN%BUILD SETUP COMPLETE.%RESET%
echo You can now proceed with building your project.
echo =======================================

:end
pause