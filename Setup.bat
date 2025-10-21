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
echo 2. Executing GameInput Redistributable Installer
echo ---------------------------------------

REM Run the installer and capture its exit code immediately
start /wait "" "Frost\vendor\GameInput\redist\GameInputRedist.msi"
set "INSTALL_ERROR_CODE=%ERRORLEVEL%"

REM Branching Logic using GOTO:

REM Check for success (0)
if "%INSTALL_ERROR_CODE%"=="0" goto :installer_continue

REM Check for restart required (3010)
if "%INSTALL_ERROR_CODE%"=="3010" goto :installer_restart_warning

REM Any other non-zero code is considered a general failure
goto :installer_failure

:installer_restart_warning
    echo.
    echo [WARNING] GameInput installer indicated a required restart (Error code 3010).
    echo Please restart your computer if you encounter issues.
    goto :installer_continue

:installer_failure
    echo.
    echo [ERROR] GameInput installer failed or was canceled (Error Code: %INSTALL_ERROR_CODE%).
    echo Please check the file path and try running the installer manually.
    REM Continue to the next step, as some users might want to try to build anyway.
    goto :installer_continue

:installer_continue
echo GameInput installer executed.
echo.

echo ---------------------------------------
echo 3. Running CMake Configuration (cmake .)
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
