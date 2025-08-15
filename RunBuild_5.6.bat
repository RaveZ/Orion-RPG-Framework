@echo off
setlocal enabledelayedexpansion

:: Define the base paths for your project and plugin
set "PLUGIN_BASE_PATH=C:\ALPHA\UEProject\Orion-RPG-Framework\Plugins\OrionRPG"
set "PACKAGE_BASE_PATH=C:\ALPHA\Build\OrionRPG-Plugin"
set "ENGINE_BASE_PATH=C:\Program Files\Epic Games"

:: Define the Unreal Engine versions to build for
set "UE_VERSIONS=UE_5.6"

echo.
echo Starting multi-version plugin build for OrionRPG...
echo.

:: Loop through each specified Unreal Engine version
for %%V in (%UE_VERSIONS%) do (
    echo ==========================================================
    echo Building for Unreal Engine version: %%V
    echo ==========================================================

    :: Set the full paths for the current version
    set "ENGINE_UAT_PATH=!ENGINE_BASE_PATH!\%%V\Engine\Build\BatchFiles\RunUAT.bat"
    set "CURRENT_PACKAGE_PATH=!PACKAGE_BASE_PATH!\%%V\OrionRPGPlugin"
    set "ZIP_FILE_PATH=!PACKAGE_BASE_PATH!\ZipFiles\OrionRPGPlugin_%%V.zip"
    
    :: Check if the Unreal Engine path exists before trying to build
    if exist "!ENGINE_UAT_PATH!" (
        echo.
        echo Building plugin with UAT from "!ENGINE_UAT_PATH!"
        echo Output will be in "!CURRENT_PACKAGE_PATH!"
        echo.
        
        :: Run the BuildPlugin command
        call "!ENGINE_UAT_PATH!" BuildPlugin -Plugin="%PLUGIN_BASE_PATH%\OrionRPG.uplugin" -targetPlatforms=Win64 -Package="!CURRENT_PACKAGE_PATH!" -Rocket
        
        :: Check the exit code of the build command
        if !errorlevel! equ 0 (
            echo.
            echo Plugin build for %%V successful.
            echo.
            
            :: Deleting Intermediate and Binaries folders from the packaged plugin directory
            echo Deleting Intermediate and Binaries folders from the packaged plugin...
            rd /s /q "!CURRENT_PACKAGE_PATH!\Intermediate"
            rd /s /q "!CURRENT_PACKAGE_PATH!\Binaries"
            
            echo Cleanup of packaged plugin complete.
            echo.
            
            :: Ensure the zip file's parent directory exists
            if not exist "!PACKAGE_BASE_PATH!\ZipFiles\" mkdir "!PACKAGE_BASE_PATH!\ZipFiles\"
            
            echo Zipping the packaged plugin to "!ZIP_FILE_PATH!"
            
            :: Use PowerShell to create the zip file. The -Force flag overwrites if it already exists.
            powershell -command "Compress-Archive -Path '!CURRENT_PACKAGE_PATH!' -DestinationPath '!ZIP_FILE_PATH!' -Force"
            
            echo.
            echo Zip file created.
            
            echo All steps complete for %%V.
            echo.
        ) else (
            echo.
            echo Plugin build for %%V failed. Cleanup and zipping skipped.
            echo.
        )
    ) else (
        echo.
        echo Unreal Engine version %%V not found at "!ENGINE_UAT_PATH!". Skipping this version.
        echo.
    )
)

echo ==========================================================
echo All versions processed.
echo ==========================================================

pause
