@echo off
setlocal
if not exist build mkdir build
gcc --version >nul 2>&1
if errorlevel 1 (
    echo GCC was not found. Install MSYS2 MinGW or add gcc to PATH.
    pause
    exit /b 1
)
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\platform.c -o build\platform.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\rpc_protocol.c -o build\rpc_protocol.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\game.c -o build\game.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\rpc_client.c -o build\rpc_client.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\rpc_server.c -o build\rpc_server.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\server_main.c -o build\server_main.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\client_main.c -o build\client_main.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\demo_main.c -o build\demo_main.o
if errorlevel 1 goto fail
gcc -std=c11 -Wall -Wextra -O2 -Iinclude -c src\concurrent_demo_main.c -o build\concurrent_demo_main.o
if errorlevel 1 goto fail
gcc -o build\server.exe build\platform.o build\rpc_protocol.o build\game.o build\rpc_client.o build\rpc_server.o build\server_main.o -lws2_32
if errorlevel 1 goto fail
gcc -o build\client.exe build\platform.o build\rpc_protocol.o build\game.o build\rpc_client.o build\rpc_server.o build\client_main.o -lws2_32
if errorlevel 1 goto fail
gcc -o build\demo.exe build\platform.o build\rpc_protocol.o build\game.o build\rpc_client.o build\rpc_server.o build\demo_main.o -lws2_32
if errorlevel 1 goto fail
gcc -o build\concurrent_demo.exe build\platform.o build\rpc_protocol.o build\game.o build\rpc_client.o build\rpc_server.o build\concurrent_demo_main.o -lws2_32
if errorlevel 1 goto fail
echo Build completed.
exit /b 0
:fail
echo Build failed.
pause
exit /b 1
