@echo off
setlocal

echo.
echo === Démarrage du Nettoyage COMPLET du Projet ===
echo.

:: -----------------------------------------------------------------------------
:: SECTION 1: NETTOYAGE DU RÉPERTOIRE RACINE
:: -----------------------------------------------------------------------------

echo.
echo --- Nettoyage du Répertoire Racine ---

:: Répertoires racine à supprimer
:: Note: Frost et Lab sont laissés ici pour être nettoyés en interne à la Section 2.
set "ROOT_DIRS_TO_REMOVE=CMakeFiles x64 Debug Release"

echo Suppression des répertoires de construction racines...
for %%d in (%ROOT_DIRS_TO_REMOVE%) do (
    if exist "%%d" (
        echo   Suppression du répertoire: %%d
        rd /s /q "%%d"
    )
)

:: Fichiers racines à supprimer
set "ROOT_FILES_TO_REMOVE=*.sln *.vcxproj *.vcxproj.filters CMakeCache.txt cmake_install.cmake"

echo Suppression des fichiers racines générés...
for %%f in (%ROOT_FILES_TO_REMOVE%) do (
    if exist "%%f" (
        echo   Suppression de %%f
        del /q "%%f"
    )
)

:: -----------------------------------------------------------------------------
:: SECTION 2: NETTOYAGE DES SOUS-RÉPERTOIRES (FROST, LAB)
:: -----------------------------------------------------------------------------

echo.
echo --- Nettoyage des Sous-répertoires (Frost, Lab) ---

set "SUB_DIRS_TO_CLEAN=Frost Lab"

:: Répertoires à supprimer à l'INTÉRIEUR des sous-répertoires (noms explicites)
set "SUB_EXPLICIT_DIRS=bin CMakeFiles Debug Release"

for %%s in (%SUB_DIRS_TO_CLEAN%) do (
    if exist "%%s" (
        echo.
        echo Démarrage du nettoyage dans: %%s
        pushd "%%s"
        
        :: 1. Suppression des répertoires de construction explicites (bin, CMakeFiles)
        for %%d in (%SUB_EXPLICIT_DIRS%) do (
            if exist "%%d" (
                echo   [%%s] Suppression du répertoire: %%d
                rd /s /q "%%d"
            )
        )
        
        :: 2. Suppression des répertoires cible CMake (*.dir)
        :: On utilise FOR /D pour gérer le caractère générique sur les répertoires
        for /d %%g in (*.dir) do (
            echo   [%%s] Suppression du répertoire cible CMake: %%g
            rd /s /q "%%g"
        )
        
        :: 3. Suppression des fichiers de construction
        set "SUB_BUILD_FILES=*.vcxproj *.vcxproj.filters cmake_install.cmake"
        for %%f in (%SUB_BUILD_FILES%) do (
            if exist "%%f" (
                echo   [%%s] Suppression de %%f
                del /q "%%f"
            )
        )
        
        popd
    )
)

:: -----------------------------------------------------------------------------
:: FINAL MESSAGE
:: -----------------------------------------------------------------------------

echo.
echo === Nettoyage COMPLET du Projet terminé ! ✨ ===
endlocal