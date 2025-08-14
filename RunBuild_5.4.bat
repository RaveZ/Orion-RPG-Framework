@echo off
call "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="C:\ALPHA\UEProject\Orion-RPG-Framework\Plugins\OrionRPG\OrionRPG.uplugin" -targetPlatforms=Win64 -Package="C:\ALPHA\Build\OrionRPG-Plugin\5.4\OrionRPGPlugin" -Rocket
pause