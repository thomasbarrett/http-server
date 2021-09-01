#ifndef TCP_SOCKET
#define TCP_SOCKET

#include <buffer.h>
#include <stdbool.h>

/**
 * tcp_socket_t is a minimal wrapper over standard library tcp sockets in non-blocking mode.
 * This interface provides a convenient and correct way to write asynchronous networking code.
 */
typedef struct tcp_socket tcp_socket_t;

/* tcp_socket_connect_cb is triggered when the socket connects. */
typedef void (*tcp_socket_connect_cb)(tcp_socket_t *sock);

/* tcp_socket_close_cb is triggered when the socket is closed. */
typedef void (*tcp_socket_close_cb)(tcp_socket_t *sock);

/* tcp_socket_error_cb is triggered when an error occurs. */
typedef void (*tcp_socket_error_cb)(tcp_socket_t *sock, int errnum);

/* tcp_socket_read_cb is triggered when the data is recieved. */
typedef void (*tcp_socket_read_cb)(tcp_socket_t *sock, buffer_view_t chunk);

/* tcp_socket_read_cb is triggered when the data is recieved. */
typedef void (*tcp_socket_end_cb)(tcp_socket_t *sock);

/* tcp_socket_drain_cb is triggered when the write queue is empty. */
typedef void (*tcp_socket_drain_cb)(tcp_socket_t *sock);

/* tcp_socket_handler_t contains all posible event handlers for a tcp_socket_t */
typedef struct tcp_socket_handler {
    tcp_socket_connect_cb on_connect;
    tcp_socket_close_cb on_close;
    tcp_socket_error_cb on_error;
    tcp_socket_read_cb on_read;
    tcp_socket_drain_cb on_drain;
    tcp_socket_drain_cb on_end;
} tcp_socket_handler_t;

/** 
 * Create a new tcp socket with the given event handler.
 * 
 * @param handler the tcp socket event handler.
 * @return the tcp socket.
 */
tcp_socket_t *tcp_socket_create(tcp_socket_handler_t handler);

/**
 * tcp_socket_connect attempts to initiate a connection to the remote `host` and `port`.
 * 
 * This function will not wait for the connection to be established. The connection status can
 * be queried at any time with the `is_connected` method. When a connection is established,
 * the on_connect event will be called. If the an error occurs, then the on_error event will be
 * called.
 * 
 * @param host the remote host
 * @param port the remote port
 * @return 0 if successful -1 otherwise
 */
int tcp_socket_connect(tcp_socket_t *sock, char *host, int port);

/**
 * tcp_socket_write attempts to write the given buffer to the given socket. 
 * 
 * If possible, write the entire buffer to the socket file descriptor and return true. If the
 * operation would block, write all remaining data to a queue and return false. The tcp_socket_poll
 * method will write and remove as much data as possible from the write queue if the file
 * descriptor becomes available for writing. Once the write queue is empty, the on_drain event will
 * be called. 
 * 
 * @param sock the socket to write to
 * @param buffer the buffer to write to the socket
 * @return true if the while buffer was written and false if some data was added to the queue.
 */
bool tcp_socket_write(tcp_socket_t *sock, buffer_view_t buffer);

/**
 * tcp_socket_destroy closes the given socket connection. 
 * @param sock the socket to be destroyed
 */
void tcp_socket_destroy(tcp_socket_t *sock);

/**
 * tcp_socket_poll attempts to connect, read, and write from the socket and calls
 * the appropiate callback if successful.
 * @param sock the socket to poll
 */
int tcp_socket_poll(tcp_socket_t *sock);

/**
 * Return true if `sock` is not yet connected and false otherwise.
 * 
 * @param sock the socket
 * @return true if not yet connected and false otherwise
 */
bool tcp_socket_is_pending(tcp_socket_t *sock);

/**
 * Return true if `sock` is connecting and false otherwise. This occurs after tcp_socket_connect
 * has been called but before the `on_connect` event is triggered.
 * 
 * @param sock the socket
 * @return true if connecting
 */
bool tcp_socket_is_connecting(tcp_socket_t *sock);

/**
 * Close the socket for writing by sending a FIN message to the peer. If the socket is open for
 * both reading and writing, then the socket will remain open for reading until the 'on_end'
 * event is triggered. If the socket is half-open for writing (a FIN packet has already been recieved),
 * then the socket will close completely.
 * 
 * @param sock the socket
 */
void tcp_socket_end(tcp_socket_t *sock);

int tcp_socket_fd(tcp_socket_t *sock);

#endif /* TCP_SOCKET */
