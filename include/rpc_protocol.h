#ifndef RPC_PROTOCOL_H
#define RPC_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define RPC_MAGIC 0x53475250u
#define RPC_VERSION 1u
#define RPC_DEFAULT_PORT 5050
#define RPC_PROGRAM_NUMBER 0x31234567u
#define RPC_SERVICE_VERSION 1u
#define RPC_NAME_SIZE 32
#define RPC_MESSAGE_SIZE 256
#define RPC_VIEW_SIZE 1024
#define RPC_HEADER_SIZE 16
#define RPC_REQUEST_SIZE 44
#define RPC_RESPONSE_SIZE 1320

typedef enum {
    RPC_PROC_JOIN = 1,
    RPC_PROC_CHOOSE_BRIDGE = 2,
    RPC_PROC_MOVE = 3,
    RPC_PROC_GET_STATE = 4,
    RPC_PROC_RESET_GAME = 5,
    RPC_PROC_SHUTDOWN = 6
} RpcProcedure;

typedef enum {
    RPC_STATUS_OK = 0,
    RPC_STATUS_ERROR = 1
} RpcStatus;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t procedure;
    uint32_t xid;
    uint32_t length;
} RpcHeader;

typedef struct {
    uint32_t player_id;
    int32_t value1;
    int32_t value2;
    char name[RPC_NAME_SIZE];
} RpcRequest;

typedef struct {
    int32_t status;
    uint32_t player_id;
    int32_t bridge;
    int32_t position;
    int32_t alive;
    int32_t finished;
    int32_t survivors;
    int32_t time_left;
    uint32_t move_sequence;
    int32_t result_code;
    char message[RPC_MESSAGE_SIZE];
    char view[RPC_VIEW_SIZE];
} RpcResponse;

void rpc_request_clear(RpcRequest *request);
void rpc_response_clear(RpcResponse *response);
uint32_t rpc_next_xid(void);
void rpc_encode_header(const RpcHeader *header, unsigned char *buffer);
int rpc_decode_header(const unsigned char *buffer, RpcHeader *header);
void rpc_encode_request(const RpcRequest *request, unsigned char *buffer);
int rpc_decode_request(const unsigned char *buffer, RpcRequest *request);
void rpc_encode_response(const RpcResponse *response, unsigned char *buffer);
int rpc_decode_response(const unsigned char *buffer, RpcResponse *response);

#endif
