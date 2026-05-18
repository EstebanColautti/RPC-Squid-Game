# RPC Squid Game: The Glass Bridge

This project implements the Glass Bridge challenge using a client-server RPC architecture over TCP sockets. Clients invoke remote procedures, while the server maintains the authoritative game state and validates every action.

## Contents

- RPC server.
- Interactive client.
- Automatic demonstration.
- Concurrent demonstration with multiple clients.
- Protocol header with program number, version, procedure number, payload size, and transaction identifier.
- Network-byte-order serialization.
- Service definition file: `rpc_interface/squid_game.x`.
- Windows build and execution scripts.
- Makefile for Linux, WSL, or MSYS2.
- Execution logs in `evidence/`.
- Execution screenshot in `screenshots/`.

## Requirements

### Windows

Install a GCC-compatible compiler, such as MSYS2 MinGW, and make sure `gcc` is available in the system `PATH`.

Recommended MSYS2 MinGW package:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
```

To open the main execution menu:

```bat
RUN_PROJECT.bat
```

Direct execution options:

```bat
RUN_AUTOMATIC_DEMO.bat
RUN_CONCURRENT_DEMO.bat
RUN_MANUAL_GAME.bat
```

### Linux, WSL, or MSYS2

```bash
make all
./build/demo
./build/concurrent_demo
```

## Manual Execution

Terminal 1:

```bash
./build/server 5050 10 90
```

Terminal 2:

```bash
./build/client 127.0.0.1 5050
```

Terminal 3:

```bash
./build/client 127.0.0.1 5050
```

## Remote Procedures

- `JOIN_PLAYER`: registers a player.
- `CHOOSE_BRIDGE`: selects bridge 1, 2, or 3.
- `MOVE_PLAYER`: moves 1 or 2 steps forward.
- `GET_STATE`: returns the visible game state.
- `RESET_GAME`: restarts the game for demonstrations.
- `SHUTDOWN`: shuts down the local demo server.

## Implemented Rules

- The game has 3 parallel bridges.
- Each bridge has N steps.
- Each step is either strong or weak.
- Step types remain hidden until tested.
- Each player starts on Side A.
- Each player chooses one bridge and cannot switch bridges afterward.
- Players can only move forward.
- Players can jump 1 or 2 steps.
- If a player lands on a weak step, the step breaks and the player dies.
- If a player reaches Side B, the player survives and the global survivor counter increases.
- If the global clock runs out, all remaining players on the bridges die.
- If another player is ahead on the same bridge, the player must wait.

## Concurrency

The server accepts concurrent connections. Each request is processed by a worker thread, and all modifications to the shared game state are protected by a mutex. This prevents inconsistent updates when multiple clients send requests at the same time.

## RPC Design Note

The project uses a custom RPC implementation over TCP sockets. The structure follows the core RPC model: service definitions, client-side stubs, a server-side dispatcher, transaction identifiers, and serialized request/response payloads.
