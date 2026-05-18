#define _POSIX_C_SOURCE 199309L
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int net_startup(void) {
#ifdef _WIN32
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0 ? 0 : -1;
#else
    return 0;
#endif
}

void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

void net_close(socket_t socket_fd) {
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
    }
}

socket_t net_create_server(int port) {
    socket_t server_fd;
    struct sockaddr_in address;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET_VALUE) {
        return INVALID_SOCKET_VALUE;
    }

#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        net_close(server_fd);
        return INVALID_SOCKET_VALUE;
    }

    if (listen(server_fd, 32) == SOCKET_ERROR) {
        net_close(server_fd);
        return INVALID_SOCKET_VALUE;
    }

    return server_fd;
}

socket_t net_connect_to(const char *host, int port) {
    socket_t client_fd;
    struct sockaddr_in address;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == INVALID_SOCKET_VALUE) {
        return INVALID_SOCKET_VALUE;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, host, &address.sin_addr) <= 0) {
        struct hostent *entry = gethostbyname(host);
        if (entry == NULL) {
            net_close(client_fd);
            return INVALID_SOCKET_VALUE;
        }
        memcpy(&address.sin_addr, entry->h_addr_list[0], (size_t)entry->h_length);
    }

    if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        net_close(client_fd);
        return INVALID_SOCKET_VALUE;
    }

    return client_fd;
}

int net_send_all(socket_t socket_fd, const void *buffer, size_t length) {
    const char *cursor = (const char *)buffer;
    size_t sent_total = 0;

    while (sent_total < length) {
        int sent = send(socket_fd, cursor + sent_total, (int)(length - sent_total), 0);
        if (sent <= 0) {
            return -1;
        }
        sent_total += (size_t)sent;
    }

    return 0;
}

int net_recv_all(socket_t socket_fd, void *buffer, size_t length) {
    char *cursor = (char *)buffer;
    size_t received_total = 0;

    while (received_total < length) {
        int received = recv(socket_fd, cursor + received_total, (int)(length - received_total), 0);
        if (received <= 0) {
            return -1;
        }
        received_total += (size_t)received;
    }

    return 0;
}

int thread_start(thread_t *thread, THREAD_RETURN (*function)(THREAD_ARG), void *argument) {
#ifdef _WIN32
    *thread = CreateThread(NULL, 0, function, argument, 0, NULL);
    return *thread != NULL ? 0 : -1;
#else
    return pthread_create(thread, NULL, function, argument) == 0 ? 0 : -1;
#endif
}

void thread_detach(thread_t thread) {
#ifdef _WIN32
    CloseHandle(thread);
#else
    pthread_detach(thread);
#endif
}

void thread_join(thread_t thread) {
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    pthread_join(thread, NULL);
#endif
}

void mutex_init(mutex_t *mutex) {
#ifdef _WIN32
    InitializeCriticalSection(mutex);
#else
    pthread_mutex_init(mutex, NULL);
#endif
}

void mutex_lock(mutex_t *mutex) {
#ifdef _WIN32
    EnterCriticalSection(mutex);
#else
    pthread_mutex_lock(mutex);
#endif
}

void mutex_unlock(mutex_t *mutex) {
#ifdef _WIN32
    LeaveCriticalSection(mutex);
#else
    pthread_mutex_unlock(mutex);
#endif
}

void mutex_destroy(mutex_t *mutex) {
#ifdef _WIN32
    DeleteCriticalSection(mutex);
#else
    pthread_mutex_destroy(mutex);
#endif
}

void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (long)(milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

uint32_t platform_process_id(void) {
#ifdef _WIN32
    return (uint32_t)GetCurrentProcessId();
#else
    return (uint32_t)getpid();
#endif
}
