#include <log.h>
#include <tcp_socket.h>

void on_connect(tcp_socket_t *sock) {
    log("connect");
    int fd = tcp_socket_fd(sock);
    write(fd, "ping", 4);
}

void on_read(tcp_socket_t *sock, buffer_t chunk) {
    log("read %.*s", chunk.length, chunk.data);
}

void on_end(tcp_socket_t *sock) {
    log("end");
    tcp_socket_end(sock);
}

void on_close(tcp_socket_t *sock) {
    log("close");
}

void on_error(tcp_socket_t *sock, int errnum) {
    log("error %s", strerror(errnum));
}

int main() {
 
    tcp_socket_handler_t handler;
    handler.on_connect = on_connect;
    handler.on_read = on_read;
    handler.on_end = on_end;
    handler.on_close = on_close;
    handler.on_error = on_error;

    tcp_socket_t *sock = tcp_socket_create(handler);
    tcp_socket_connect(sock, "127.0.0.1", 8000);
   
    while (1) {
       tcp_socket_poll(sock);
    } 

    tcp_socket_destroy(sock);
}

