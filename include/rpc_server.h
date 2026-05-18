#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "platform.h"

typedef struct {
    int port;
    int steps;
    int duration_seconds;
    unsigned int seed;
} RpcServerConfig;

int rpc_server_start(const RpcServerConfig *config);
int rpc_server_start_background(const RpcServerConfig *config, thread_t *server_thread);

#endif
