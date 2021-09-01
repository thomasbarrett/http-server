#ifndef TCP_H
#define TCP_H

/**
 * 
 */

#include <netinet/in.h>
#include <buffer.h>

/**
 * tcp_client_t manages a tcp connection to a single client. Each client has an address and
 */
typedef struct tcp_client tcp_client_t;

/**
 * tcp_server_t manages a tcp socket in the listening state and a list of clients.
 */
typedef struct tcp_server tcp_server_t;

typedef void (*tcp_connect_cb)(tcp_server_t *server, tcp_client_t *client);
typedef void (*tcp_close_cb)(tcp_server_t *server, tcp_client_t *client);
typedef void (*tcp_read_cb)(tcp_server_t *server, tcp_client_t *client, buffer_t chunk);
typedef void (*tcp_error_cb)(tcp_server_t *server, tcp_client_t *client, int errnum);

/**
 * tcp_server_create creates and returns a new tcp server with the given callback functions.
 */
tcp_server_t* tcp_server_create(tcp_connect_cb on_connect, tcp_close_cb on_close, tcp_read_cb on_read, tcp_error_cb on_error);

/**
 * tcp_server_destroy cleantly disconnects all active clients and frees all memory associated with the
 * server.
 */
void tcp_server_destroy(tcp_server_t *server);

/**
 * Poll the listening file descriptor and the file descriptor of all client connections. If
 * an incoming connection is recieved, the on_connect callback is called. If any clients
 * disconnect, the on_close callback is called. If any of the client file descriptors have data
 * available to read, the on_read callback is called.
 *
 * @param self: the server
 * @return -1 if an error occurs and 0 otherwise.
 */
int tcp_server_poll(tcp_server_t *self);

/**
 * Start listening for incoming connections on the specified port. If an error occurs, return -1 
 * and set errno to the appropriate error code.
 *
 * @param self: the server:
 * @param port: the port on which to listen for incoming connections
 * @param backlog: the maximum number of queued connections
 * @return -1 if an error occurs and 0 otherwise.
 */
int tcp_server_listen(tcp_server_t *self, int port, int backlog);

/**
 * tcp_client_addr returns the sockaddr_in corresponding to the client
 */
struct sockaddr_in tcp_client_addr(tcp_client_t *client);

/**
 * Close the client connection at the start of the next call to tcp_server_poll.
 *
 * @param self: the client
 */
void tcp_server_close_client(tcp_server_t *self, tcp_client_t *client);

/**
 * Set the client's extra data field. The extra data field should be used for 
 * adding application specific data to a tcp_client. The extra data field
 * should be properly destroyed in the on_close callback.
 *
 * @param self: the client
 * @param data: the extra data
 */
void tcp_client_set_data(tcp_client_t *self, void *data);

/**
 * Return the client's extra data field or NULL if no such field was ever set.
 *
 * @param self: the client
 * @return the extra data
 */
void* tcp_client_data(tcp_client_t *self);

int tcp_client_fd(tcp_client_t *self);

#endif /* TCP_H */

