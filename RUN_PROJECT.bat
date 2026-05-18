@echo off
setlocal
cd /d "%~dp0"

echo RPC Squid Game - Execution Menu
echo.
echo 1. Automatic demo
echo 2. Concurrent demo
echo 3. Manual game: server and two clients
echo 4. Build only
echo.
set /p option=Select an option: 

if "%option%"=="1" call RUN_AUTOMATIC_DEMO.bat
if "%option%"=="2" call RUN_CONCURRENT_DEMO.bat
if "%option%"=="3" call RUN_MANUAL_GAME.bat
if "%option%"=="4" call BUILD_WINDOWS.bat

echo.
pause
