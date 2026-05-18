#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef SOCKET socket_t;
typedef HANDLE thread_t;
typedef CRITICAL_SECTION mutex_t;
#define INVALID_SOCKET_VALUE INVALID_SOCKET
#define CLOSE_SOCKET closesocket
#define THREAD_RETURN DWORD WINAPI
#define THREAD_ARG LPVOID
#else
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
typedef int socket_t;
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
#define INVALID_SOCKET_VALUE (-1)
#define SOCKET_ERROR (-1)
#define CLOSE_SOCKET close
#define THREAD_RETURN void *
#define THREAD_ARG void *
#endif

int net_startup(void);
void net_cleanup(void);
socket_t net_create_server(int port);
socket_t net_connect_to(const char *host, int port);
int net_send_all(socket_t socket_fd, const void *buffer, size_t length);
int net_recv_all(socket_t socket_fd, void *buffer, size_t length);
void net_close(socket_t socket_fd);
int thread_start(thread_t *thread, THREAD_RETURN (*function)(THREAD_ARG), void *argument);
void thread_detach(thread_t thread);
void thread_join(thread_t thread);
void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);
void mutex_destroy(mutex_t *mutex);
void sleep_ms(int milliseconds);
uint32_t platform_process_id(void);

#endif
