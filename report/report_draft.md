# RPC Squid Game: The Glass Bridge

## Introduction

This project implements the Glass Bridge challenge using a client-server RPC architecture. The server maintains the authoritative game state, and clients interact with it by invoking remote procedures.

The system models each relevant action as a remote procedure: joining the game, selecting a bridge, moving forward, and requesting the current visible state. This design keeps the client simple while ensuring that all game rules are enforced consistently by the server.

## Problem Description

The game contains three parallel bridges. Each bridge has N steps, and every step is either strong or weak. Step types are hidden until they are tested by a player. A player starts on Side A, selects one bridge, and can only move forward on that same bridge. A move can advance one or two steps. If the destination step is weak, it breaks and the player dies. If the player reaches Side B, the player survives and the global survivor counter increases. If the global clock reaches zero, all remaining players die.

## RPC Design

The system exposes the following remote procedures:

| Procedure | Purpose |
|---|---|
| `JOIN_PLAYER` | Registers a new player and returns a player identifier. |
| `CHOOSE_BRIDGE` | Assigns the selected bridge to a player. |
| `MOVE_PLAYER` | Requests a jump of one or two steps. |
| `GET_STATE` | Returns the visible state of the game. |
| `RESET_GAME` | Resets the game for demonstrations. |
| `SHUTDOWN` | Stops the local demo server. |

The RPC protocol includes a header with a magic number, protocol version, procedure number, transaction identifier, and payload size. The transaction identifier is echoed by the server and validated by the client, ensuring that each response matches the corresponding request.

## Client and Server Architecture

The server stores the bridges, hidden step types, visible tested steps, broken steps, players, global clock, survivor counter, and movement sequence. Clients never modify this information directly; they only send requests to the server.

The client side contains stub functions such as `rpc_join_player`, `rpc_choose_bridge`, `rpc_move_player`, and `rpc_get_state`. These functions encapsulate socket communication and allow the rest of the client program to behave as if it were calling local procedures.

The server side contains a dispatcher that receives the procedure number from the RPC header and invokes the corresponding game function. Each request is handled by a worker thread, while shared game state is protected by a mutex.

## Execution Example

Recommended command:

```bash
./build/concurrent_demo
```

On Windows:

```bat
RUN_CONCURRENT_DEMO.bat
```

The concurrent demonstration shows several clients joining the game, selecting bridges, attempting to move at the same time, waiting when another player is ahead, dying when a weak step breaks, and updating the shared visible state.

## Guiding Questions

### 1. How did you ensure consistent ordering of moves?

The server is the only component allowed to modify the game state. Even when multiple clients send RPC calls at the same time, each game-changing operation is protected by a mutex. Therefore, only one thread can validate and apply a move at a time. The server also increments a global `move_sequence` value every time a valid movement is executed, which provides a clear order of accepted moves.

### 2. Did clients ever act on stale information?

A client may display information that becomes outdated after another client moves. However, clients are not trusted to make final decisions. Every move is revalidated by the server using the current state. The server checks whether the player is alive, whether the clock is still running, whether the jump is valid, whether another player is ahead, and whether the target step has already been broken. Because of this, stale client information cannot corrupt the game state.

### 3. Where did race conditions appear?

Race conditions could appear when two players move at nearly the same time, when multiple requests update the survivor counter, when a weak step breaks while another player attempts to use related state, when the clock expires during movement, or when several clients request the state while a move is being processed. The solution was to centralize the game state on the server and protect all critical sections with a mutex.

## Conclusion

The final system satisfies the Glass Bridge challenge using an RPC client-server architecture. The implementation demonstrates remote procedure calls, server-side state authority, concurrent client handling, transaction identifiers, serialized request and response payloads, and protected access to shared game data.
