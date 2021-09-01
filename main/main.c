#include <log.h>
#include <tcp.h>

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define TCP_PORT 8000
#define TCP_QUEUE 16


void on_connect(tcp_server_t *server, tcp_client_t *client) {
    struct sockaddr_in addr = tcp_client_addr(client);
    log("[%s:%d] client connected", inet_ntoa(addr.sin_addr), addr.sin_port);
}

void on_close(tcp_server_t *server, tcp_client_t *client) {
    struct sockaddr_in addr = tcp_client_addr(client);
    log("[%s:%d] client disconnected", inet_ntoa(addr.sin_addr), addr.sin_port);
}

void on_read(tcp_server_t *server, tcp_client_t *client, buffer_t chunk) {
    struct sockaddr_in addr = tcp_client_addr(client);
    log("[%s:%d] sent %d characters", inet_ntoa(addr.sin_addr), addr.sin_port, chunk.length); 
    write(tcp_client_fd(client), chunk.data, chunk.length);
    shutdown(tcp_client_fd(client), SHUT_WR);
}

void on_error(tcp_server_t *server, tcp_client_t *client, int errnum) {
    struct sockaddr_in addr = tcp_client_addr(client);
    log("[%s:%d] failed with error %s", inet_ntoa(addr.sin_addr), addr.sin_port, strerror(errnum)); 
}

int main() {
 
    tcp_server_t *server = tcp_server_create(on_connect, on_close, on_read, on_error);

    int res = tcp_server_listen(server, TCP_PORT, TCP_QUEUE);
    if (res != 0) {
        log("error: %s", strerror(errno));
        tcp_server_destroy(server);
        return 1;
    }

    log("\x1b[31mListening\x1b[0m on port %d", TCP_PORT);

    while (1) {
        int res = tcp_server_poll(server);
        if (res != 0) {
            log("error: %s", strerror(errno));
        }
    } 

    tcp_server_destroy(server);
}

