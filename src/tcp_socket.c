#include <tcp_socket.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define DEFAULT_CHUNK_SIZE 1024

typedef struct tcp_socket {
    int fd;
    tcp_socket_handler_t handler;
    struct sockaddr_in remote_addr;
    int connected;
    int open_read;
    int open_write;
} tcp_socket_t;

tcp_socket_t *tcp_socket_create(tcp_socket_handler_t handler) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) return NULL;

    int res = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (res == -1) {
        close(fd);
        return NULL;
    }

    res = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
    if (res != 0) {
        close(fd);
        return NULL;
    }

    res = setsockopt(fd, SOL_SOCKET, SO_LINGER, &(struct linger){1, 0}, sizeof(struct linger));
    if (res != 0) {
        close(fd);
        return NULL;
    }

    tcp_socket_t *sock = calloc(1, sizeof(tcp_socket_t));
    sock->fd = fd;
    sock->handler = handler;
    return sock;
}

int tcp_socket_connect(tcp_socket_t *sock, char *host, int port) {
    struct sockaddr_in addr;
    int res = inet_pton(AF_INET, host, &(addr.sin_addr));
    if (res == 0) return -1;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    sock->remote_addr = addr;

    struct sockaddr *remote_addr = (struct sockaddr*) &sock->remote_addr;
    res = connect(sock->fd, remote_addr, sizeof(struct sockaddr_in));
    if (res == -1 && errno != EINPROGRESS) return -1;
    return 0;
}

int tcp_socket_poll(tcp_socket_t *sock) {
    
    /* check if socket is connected */
    if (sock->connected == 0) {
        struct sockaddr_in peer_addr;
        size_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(sock->fd, &peer_addr, &addr_size);
        if (res == 0) {
            sock->connected = 1;
            sock->open_read = 1;
            sock->open_write = 1;
            sock->handler.on_connect(sock);
        }
    }
    
    /* check for errors */
    int error;
    int res = getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, &error, &(size_t){sizeof(int)});
    if (res < 0) {
        return -1;
    }

    if (error != 0) {
        sock->handler.on_error(sock, error);
    }

    /* check if socket is available for reading */
    if (sock->connected == 1) {
        short events = 0;
        events |= sock->open_read ? POLLIN: 0;
        struct pollfd fds = {sock->fd, events, 0};
        int res = poll(&fds, 1, 0);
        if (res == -1) return -1;
        if (res > 0) {
            if (fds.revents & POLLIN) {
                uint8_t chunk[DEFAULT_CHUNK_SIZE] = {0};
                ssize_t nread = read(sock->fd, chunk, DEFAULT_CHUNK_SIZE);
                if (nread < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    sock->handler.on_error(sock, errno);
                } else if (nread == 0) {
                    sock->open_read = 0;
                    if (sock->open_write == 1) {
                        sock->handler.on_end(sock);
                    } else {
                        close(sock->fd);
                        sock->handler.on_close(sock);
                    }
                } else {
                    sock->handler.on_read(sock, (buffer_t){chunk, nread});
                }
            }
        }
    }
}

int tcp_socket_fd(tcp_socket_t *sock) {
    return sock->fd;
}

void tcp_socket_end(tcp_socket_t *sock) {
    if (sock->open_write == 1) {
        shutdown(sock->fd, SHUT_WR);
    }
    if (sock->open_read == 0) {
        close(sock->fd);
        sock->handler.on_close(sock);
    }
}
