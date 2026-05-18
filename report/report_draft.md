# RPC Squid Game: The Glass Bridge

## Introduction

This project implements the RPC Squid Game glass bridge challenge using a client-server architecture. The server owns the authoritative game state and the clients interact with it through remote procedure calls.

Each operation is modeled as a remote procedure: joining the game, choosing a bridge, moving, and requesting the current state. This keeps the client logic simple while the communication is performed through the network.

## Problem Description

The game has three parallel bridges. Each bridge has N steps. Every step is either strong or weak. Step types are hidden until tested. A player starts on Side A, chooses one bridge, and then can only move forward on that same bridge. A movement can jump one or two steps. If the target step is weak, it breaks and the player dies. If the player reaches Side B, the player survives and the global survivor counter increases. If the global clock reaches zero, every remaining player dies.

## RPC Design

The system exposes these remote procedures:

| Procedure | Purpose |
|---|---|
| `JOIN_PLAYER` | Registers a new player and returns a player id. |
| `CHOOSE_BRIDGE` | Assigns the selected bridge to a player. |
| `MOVE_PLAYER` | Requests a jump of one or two steps. |
| `GET_STATE` | Returns the visible state of the game. |
| `RESET_GAME` | Resets the game for demonstrations. |
| `SHUTDOWN` | Stops the local demo server. |

The RPC protocol includes a header with a magic number, protocol version, procedure number, transaction id, and payload size. The transaction id is echoed by the server and checked by the client, so the client can verify that the response corresponds to the request it sent.

## Client and Server Architecture

The server stores the bridges, hidden step types, visible tested steps, broken steps, players, global clock, survivor counter, and movement sequence. Clients never modify this information directly. They only send requests to the server.

The client side has stub functions such as `rpc_join_player`, `rpc_choose_bridge`, `rpc_move_player`, and `rpc_get_state`. These functions hide the socket communication and allow the rest of the client code to behave as if it were calling normal local procedures.

The server side has a dispatcher that receives the procedure number from the RPC header and calls the correct game function. Each request is handled by a worker thread, while shared game state is protected by a mutex.

## Execution Example

Recommended command:

```bash
./build/concurrent_demo
```

On Windows:

```bat
RUN_CONCURRENT_DEMO.bat
```

The demo shows several clients joining, choosing bridges, trying to move at the same time, waiting when another player is ahead, dying when a weak step breaks, and updating the global visible state.

## Guiding Questions

### 1. How did you ensure consistent ordering of moves?

The server is the only component allowed to modify the game state. Even though multiple clients can send RPC calls at the same time, every game-changing operation is protected by a mutex. This means only one thread can validate and apply a move at a time. The server also increments a global `move_sequence` every time a valid movement is executed. This sequence provides a clear order of accepted moves.

### 2. Did clients ever act on stale information?

A client may display information that becomes old after another client moves. However, clients are not trusted to make final decisions. Every move is revalidated on the server using the current state. For example, the server checks whether the player is still alive, whether the clock is still running, whether the selected jump is valid, whether another player is ahead, and whether the target step is already broken. Because of this, stale client information does not corrupt the game.

### 3. Where did race conditions appear?

Race conditions could appear when two players move at nearly the same time, when two requests update the survivor counter, when a weak step breaks while another player tries to use it, when the clock expires during movement, or when several clients request the state while a move is being processed. The solution was to centralize the game state on the server and protect all critical sections with a mutex.

## Conclusion

The final system satisfies the glass bridge challenge with an RPC client-server architecture. The implementation demonstrates remote procedure calls, server-side state authority, concurrent client handling, transaction identifiers, serlized request and response payloads, and protected access to shared game data.
