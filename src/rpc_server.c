#include "rpc_server.h"
#include "rpc_protocol.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/select.h>
#include <sys/time.h>
#endif

static GameState g_game;
static volatile int g_running = 0;
static socket_t g_server_socket = INVALID_SOCKET_VALUE;

typedef struct {
    socket_t client_socket;
} ClientJob;

typedef struct {
    RpcServerConfig config;
} ServerThreadArgs;

static void make_error_response(RpcResponse *response, const char *message) {
    rpc_response_clear(response);
    response->status = RPC_STATUS_ERROR;
    snprintf(response->message, RPC_MESSAGE_SIZE, "%s", message);
}

static void dispatch_request(const RpcHeader *header, const RpcRequest *request, RpcResponse *response) {
    if (header->magic != RPC_MAGIC || header->version != RPC_VERSION || header->length != RPC_REQUEST_SIZE) {
        make_error_response(response, "Invalid RPC header.");
        return;
    }

    switch (header->procedure) {
        case RPC_PROC_JOIN:
            game_join(&g_game, request->name, response);
            break;
        case RPC_PROC_CHOOSE_BRIDGE:
            game_choose_bridge(&g_game, request->player_id, request->value1, response);
            break;
        case RPC_PROC_MOVE:
            game_move(&g_game, request->player_id, request->value1, response);
            break;
        case RPC_PROC_GET_STATE:
            game_get_state(&g_game, request->player_id, response);
            break;
        case RPC_PROC_RESET_GAME:
            game_reset(&g_game, request->value1, request->value2, 42u, response);
            break;
        case RPC_PROC_SHUTDOWN:
            response->status = RPC_STATUS_OK;
            snprintf(response->message, RPC_MESSAGE_SIZE, "Server shutdown requested.");
            game_get_state(&g_game, request->player_id, response);
            snprintf(response->message, RPC_MESSAGE_SIZE, "Server shutdown requested.");
            g_running = 0;
            if (g_server_socket != INVALID_SOCKET_VALUE) {
                net_close(g_server_socket);
                g_server_socket = INVALID_SOCKET_VALUE;
            }
            break;
        default:
            make_error_response(response, "Unknown remote procedure.");
            break;
    }
}

static THREAD_RETURN handle_client(THREAD_ARG argument) {
    ClientJob *job = (ClientJob *)argument;
    socket_t client_socket = job->client_socket;
    RpcHeader header;
    RpcHeader response_header;
    RpcRequest request;
    RpcResponse response;
    unsigned char header_buffer[RPC_HEADER_SIZE];
    unsigned char request_buffer[RPC_REQUEST_SIZE];
    unsigned char response_buffer[RPC_RESPONSE_SIZE];

    free(job);
    rpc_request_clear(&request);
    rpc_response_clear(&response);

    if (net_recv_all(client_socket, header_buffer, RPC_HEADER_SIZE) != 0) {
        net_close(client_socket);
#ifdef _WIN32
        return 0;
#else
        return NULL;
#endif
    }

    rpc_decode_header(header_buffer, &header);

    if (header.length != RPC_REQUEST_SIZE || net_recv_all(client_socket, request_buffer, RPC_REQUEST_SIZE) != 0) {
        make_error_response(&response, "Could not read RPC request body.");
    } else {
        rpc_decode_request(request_buffer, &request);
        dispatch_request(&header, &request, &response);
    }

    response_header.magic = RPC_MAGIC;
    response_header.version = RPC_VERSION;
    response_header.procedure = header.procedure;
    response_header.xid = header.xid;
    response_header.length = RPC_RESPONSE_SIZE;
    rpc_encode_header(&response_header, header_buffer);
    rpc_encode_response(&response, response_buffer);

    net_send_all(client_socket, header_buffer, RPC_HEADER_SIZE);
    net_send_all(client_socket, response_buffer, RPC_RESPONSE_SIZE);
    net_close(client_socket);

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int rpc_server_start(const RpcServerConfig *config) {
    int port = config != NULL && config->port > 0 ? config->port : RPC_DEFAULT_PORT;
    int steps = config != NULL && config->steps > 0 ? config->steps : 10;
    int duration = config != NULL && config->duration_seconds > 0 ? config->duration_seconds : 90;
    unsigned int seed = config != NULL && config->seed != 0u ? config->seed : 42u;

    if (net_startup() != 0) {
        fprintf(stderr, "Network startup failed.\n");
        return 1;
    }

    memset(&g_game, 0, sizeof(g_game));
    game_initlize(&g_game, steps, duration, seed);

    g_server_socket = net_create_server(port);
    if (g_server_socket == INVALID_SOCKET_VALUE) {
        fprintf(stderr, "Could not start server on port %d.\n", port);
        game_destroy(&g_game);
        net_cleanup();
        return 1;
    }

    g_running = 1;
    printf("RPC Squid Game server running on port %d\n", port);
    printf("Program=0x%08x Version=%u Steps=%d Duration=%ds\n", RPC_PROGRAM_NUMBER, RPC_SERVICE_VERSION, steps, duration);

    while (g_running) {
        struct sockaddr_in client_address;
#ifdef _WIN32
        int address_length = sizeof(client_address);
#else
        socklen_t address_length = sizeof(client_address);
#endif
        fd_set read_set;
        struct timeval timeout;
        FD_ZERO(&read_set);
        FD_SET(g_server_socket, &read_set);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int ready = select((int)(g_server_socket + 1), &read_set, NULL, NULL, &timeout);
        if (ready <= 0) {
            continue;
        }
        socket_t client_socket = accept(g_server_socket, (struct sockaddr *)&client_address, &address_length);
        if (client_socket == INVALID_SOCKET_VALUE) {
            continue;
        }

        ClientJob *job = (ClientJob *)malloc(sizeof(ClientJob));
        if (job == NULL) {
            net_close(client_socket);
            continue;
        }
        job->client_socket = client_socket;

        thread_t worker;
        if (thread_start(&worker, handle_client, job) == 0) {
            thread_detach(worker);
        } else {
            free(job);
            net_close(client_socket);
        }
    }

    if (g_server_socket != INVALID_SOCKET_VALUE) {
        net_close(g_server_socket);
        g_server_socket = INVALID_SOCKET_VALUE;
    }
    game_destroy(&g_game);
    net_cleanup();
    printf("RPC Squid Game server stopped.\n");
    return 0;
}

static THREAD_RETURN server_thread_main(THREAD_ARG argument) {
    ServerThreadArgs *args = (ServerThreadArgs *)argument;
    rpc_server_start(&args->config);
    free(args);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int rpc_server_start_background(const RpcServerConfig *config, thread_t *server_thread) {
    ServerThreadArgs *args = (ServerThreadArgs *)malloc(sizeof(ServerThreadArgs));
    if (args == NULL) {
        return -1;
    }
    if (config != NULL) {
        args->config = *config;
    } else {
        args->config.port = RPC_DEFAULT_PORT;
        args->config.steps = 10;
        args->config.duration_seconds = 90;
        args->config.seed = 42u;
    }
    return thread_start(server_thread, server_thread_main, args);
}
