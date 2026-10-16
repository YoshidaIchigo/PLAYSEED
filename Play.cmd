@echo off
setlocal
set "PLAYSEED_UE=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set "PLAYSEED_PROJECT=%~dp0PLAYSEED\PLAYSEED.uproject"
if not exist "%PLAYSEED_UE%" (
  echo Unreal Engine 5.8 was not found at the expected installation path.
  pause
  exit /b 1
)
if not exist "%PLAYSEED_PROJECT%" (
  echo PLAYSEED.uproject was not found beside this launcher.
  pause
  exit /b 1
)
start "" "%PLAYSEED_UE%" "%PLAYSEED_PROJECT%" -game -windowed -ResX=1280 -ResY=720
endlocal
