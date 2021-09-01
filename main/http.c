#include <http.h>
#include <log.h>
#include <tcp.h>
#include <buffer.h>
#include <array.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>

#define TCP_PORT 8000
#define TCP_QUEUE 16

typedef struct http_client {
    time_t connect_time;
    buffer_t read_buf;
} http_client_t;

void on_connect(tcp_server_t *server, tcp_client_t *client) {
    struct sockaddr_in addr = tcp_client_addr(client);
    log("[%s:%d] client connected", inet_ntoa(addr.sin_addr), addr.sin_port);
    http_client_t *http_client = malloc(sizeof(http_client_t));
    assert(http_client != NULL && "out of memory");
    http_client->connect_time = time(NULL);
    http_client->read_buf = buffer_create(0);
    tcp_client_set_data(client, http_client);
}

void on_close(tcp_server_t *server, tcp_client_t *client) {
    struct sockaddr_in addr = tcp_client_addr(client);
    log("[%s:%d] client disconnected", inet_ntoa(addr.sin_addr), addr.sin_port);
    http_client_t *http_client = tcp_client_data(client);
    buffer_destroy(http_client->read_buf);
    free(http_client);
}

void on_read(tcp_server_t *server, tcp_client_t *client, buffer_t chunk) {
    struct sockaddr_in addr = tcp_client_addr(client);
    http_client_t *http_client = tcp_client_data(client);
    buffer_append(&http_client->read_buf, chunk);
    http_request_t *req = http_request_create();
    long len = parse_http_request(http_client->read_buf, req);
    if (len == HTTP_PARSE_ERROR) {
        log("error: invalid http request");

    } else if (len != HTTP_PARSE_INCOMPLETE) {
    
        http_headers_t *req_headers = http_request_get_headers(req);

        log("[%s:%d] %s %s", inet_ntoa(addr.sin_addr), addr.sin_port, http_request_method(req), http_request_uri(req)); 

        /* remove request from read buffer */
        buffer_splice(&http_client->read_buf, len);

        char *version = http_request_version(req);

        bool is_http_1_0 = strcmp(version, "HTTP/1.0") == 0;
        bool is_http_1_1 = strcmp(version, "HTTP/1.1") == 0;
        http_response_t *res = http_response_create();
        http_headers_t *res_headers = http_response_get_headers(res);
        
        if (!is_http_1_0 && !is_http_1_1) {
            http_response_set_status(res, 505);
            http_headers_set(res_headers, "Content-Length", "0");
            buffer_t head = http_response_write_head(res);
            write(tcp_client_fd(client), head.data, head.length);
            buffer_destroy(head);
        } else if (is_http_1_0) {
            char *connection = http_headers_get(req_headers, "Connection");
            bool close = connection == NULL || strcmp(connection, "keep-alive") != 0;
            if (close) {
                http_headers_set(res_headers, "Connection", "close");
            } else {
                http_headers_set(res_headers, "Connection", "keep-alive");
                http_headers_set(res_headers, "Content-Length", "0");
            }
            buffer_t head = http_response_write_head(res);
            write(tcp_client_fd(client), head.data, head.length);
            buffer_destroy(head);
            if (close) tcp_server_close_client(server, client);
        } else if (is_http_1_1) {
            char *connection = http_headers_get(req_headers, "Connection");
            bool close = connection != NULL && strcmp(connection, "close") == 0;
            if (close) {
                http_headers_set(res_headers, "Connection", "close");
            } else {
                http_headers_set(res_headers, "Connection", "keep-alive");
                http_headers_set(res_headers, "Content-Length", "0");
            }
            buffer_t head = http_response_write_head(res);
            write(tcp_client_fd(client), head.data, head.length);
            buffer_destroy(head);
            if (close) tcp_server_close_client(server, client);
        }

        http_response_destroy(res);
    }
    http_request_destroy(req);
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

    log("Listening on port %d", TCP_PORT);

    while (1) {
        int res = tcp_server_poll(server);
        if (res != 0) {
            log("error: %s", strerror(errno));
        }
    } 

    http_request_create();
    tcp_server_destroy(server);
}
