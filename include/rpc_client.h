#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include "rpc_protocol.h"

int rpc_call(const char *host, int port, uint16_t procedure, const RpcRequest *request, RpcResponse *response);
int rpc_join_player(const char *host, int port, const char *name, RpcResponse *response);
int rpc_choose_bridge(const char *host, int port, uint32_t player_id, int bridge, RpcResponse *response);
int rpc_move_player(const char *host, int port, uint32_t player_id, int jump, RpcResponse *response);
int rpc_get_state(const char *host, int port, uint32_t player_id, RpcResponse *response);
int rpc_reset_game(const char *host, int port, int steps, int duration, RpcResponse *response);
int rpc_shutdown_server(const char *host, int port, RpcResponse *response);
void print_rpc_response(const char *title, const RpcResponse *response);

#endif
