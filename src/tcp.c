#include <tcp.h>
#include <array.h>
#include <log.h>

#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_CLIENT_CAPACITY 16
#define DEFAULT_CHUNK_SIZE 1024

typedef struct tcp_server {
    int listen_fd;
    array_t *fds;
    array_t *clients;
    tcp_error_cb on_error;
    tcp_connect_cb on_connect;
    tcp_close_cb on_close;
    tcp_read_cb on_read;
    bool closed;
} tcp_server_t;

typedef struct tcp_client {
    struct sockaddr_in addr;
    int fd;
    void *data;
    bool closed;
} tcp_client_t ;

tcp_client_t *tcp_client_create(struct sockaddr_in addr, int fd) {
   tcp_client_t *res = calloc(1, sizeof(tcp_client_t));
   res->addr = addr;
   res->fd = fd;
   return res;
}

tcp_server_t* tcp_server_create(tcp_connect_cb on_connect, tcp_close_cb on_close, tcp_read_cb on_read, tcp_error_cb on_error) {
    tcp_server_t *server = calloc(1, sizeof(tcp_server_t));
    assert(on_connect != NULL);
    assert(on_close != NULL);
    assert(on_read != NULL);
    assert(on_error != NULL);
    server->listen_fd = -1;
    server->fds = array_create(sizeof(struct pollfd), 1 + DEFAULT_CLIENT_CAPACITY);
    server->clients = array_create(sizeof(tcp_client_t*), DEFAULT_CLIENT_CAPACITY);
    server->on_connect = on_connect;
    server->on_close = on_close;
    server->on_read = on_read;
    server->on_error = on_error;
    return server;
}

static tcp_client_t* get_client(tcp_server_t *server, int i) {
    return *(tcp_client_t**) array_get(server->clients, i);
}

static void add_client(tcp_server_t *server, tcp_client_t *client) {
    array_add(server->fds, &(struct pollfd){client->fd, POLLIN, 0});
    array_add(server->clients, &client);
}

static void remove_client(tcp_server_t *server, int i) {
    array_remove(server->fds, i + 1);
    array_remove(server->clients, i);
}

static void remove_closed_clients(tcp_server_t *server) {
    for (size_t i = 0; i < array_size(server->clients); i++) {
        tcp_client_t *client = get_client(server, i);
        if (client->closed) {
            remove_client(server, i--);
        }
    }
}

int tcp_server_listen(tcp_server_t *server, int port, int backlog) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        return -1;
    }

    int res = fcntl(listen_fd, F_SETFL, O_NONBLOCK);
    if (res == -1) {
        close(listen_fd);
        return -1;
    }

    res = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
    if (res != 0) {
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
  
    socklen_t in_addr_size = sizeof(struct sockaddr_in);

    res = bind(listen_fd, (struct sockaddr*) &server_addr, in_addr_size);
    if (res != 0) {
        close(listen_fd);
        return -1;
    }

    res = listen(listen_fd, backlog);
    if (res != 0) {
        close(server->listen_fd);
        return -1;
    }

    array_add(server->fds, &(struct pollfd){listen_fd, POLLIN, 0});
    server->listen_fd = listen_fd;
    return 0;
}

void tcp_server_destroy(tcp_server_t *server) {
    assert(server != NULL);
    for (size_t i = 0; i < array_size(server->clients); i++) {
        tcp_client_t *client = get_client(server, i);
        close(client->fd);
        server->on_close(server, client);
    }
    array_destroy(server->clients, NULL);
    array_destroy(server->fds, NULL);
}

int tcp_server_poll(tcp_server_t *server) {
    assert(server != NULL);
    
    if (server->closed) {
        remove_closed_clients(server);
        server->closed = false;
    }

    int res = poll(array_data(server->fds), array_size(server->fds), 0);
    if (res <= 0) return res;
    for (size_t i = 0; i < array_size(server->fds); i++) {
        struct pollfd *fdi = array_get(server->fds, i);
        if (fdi->revents == 0) continue;
        if (fdi->fd == server->listen_fd) {
            struct sockaddr_in client_addr = {0};
            socklen_t in_addr_size = sizeof(struct sockaddr_in);
            int client_fd = accept(server->listen_fd, (struct sockaddr*) &client_addr, &in_addr_size);
            if (client_fd < 0) {
                log(strerror(errno));
            } else {
                tcp_client_t *client = tcp_client_create(client_addr, client_fd);
                fcntl(client_fd, F_SETFL, O_NONBLOCK);
                add_client(server, client);
                server->on_connect(server, client);
            }
        } else {
            tcp_client_t *client = get_client(server, i - 1);
            if (client->closed) continue;
            uint8_t chunk[DEFAULT_CHUNK_SIZE] = {0};
            ssize_t nread = read(fdi->fd, chunk, DEFAULT_CHUNK_SIZE);
            if (nread < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                server->on_error(server, client, errno);
                tcp_server_close_client(server, client);
            } else if (nread == 0) {
                tcp_server_close_client(server, client);
            } else {
                server->on_read(server, client, (buffer_t){chunk, nread});
            }
        }
    }
    return 0;
}

struct sockaddr_in tcp_client_addr(tcp_client_t *client) {
	return client->addr;
}

void tcp_client_set_data(tcp_client_t *self, void *data) {
    self->data = data;
}

void* tcp_client_data(tcp_client_t *self) {
    return self->data;
}

int tcp_client_fd(tcp_client_t *self) {
    return self->fd;
}

void tcp_server_close_client(tcp_server_t *server, tcp_client_t *self) {
    close(self->fd);
    server->on_close(server, self);
    self->closed = true;
    server->closed = true;
}
