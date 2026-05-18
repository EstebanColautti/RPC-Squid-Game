@echo off
setlocal
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
:menu
cls
echo RPC Squid Game - C Project
echo.
echo 1. Automatic demo
echo 2. Concurrent demo
echo 3. Manual server + two clients
echo 4. Exit
echo.
set /p option=Choose an option:
if "%option%"=="1" build\demo.exe & pause & goto menu
if "%option%"=="2" build\concurrent_demo.exe & pause & goto menu
if "%option%"=="3" goto manual
if "%option%"=="4" exit /b 0
goto menu
:manual
start "RPC Squid Game Server" cmd /k build\server.exe 5050 10 90
timeout /t 1 >nul
start "RPC Squid Game Client 1" cmd /k build\client.exe 127.0.0.1 5050
start "RPC Squid Game Client 2" cmd /k build\client.exe 127.0.0.1 5050
pause
goto menu
