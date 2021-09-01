#ifndef HTTP_H
#define HTTP_H

#include <buffer.h>
#include <array.h>
#include <stdint.h>

#define HTTP_PARSE_INCOMPLETE -1
#define HTTP_PARSE_ERROR -2

/**
 * The http_request_t struct represents a single http request message. It can be parsed from a buffer
 * and its fields can be queried with the functions defined below.
 */
typedef struct http_request http_request_t;
typedef struct http_response http_response_t;
typedef array_t http_headers_t;

/**
 * Create an empty http_request. The resulting request can be passed as a parameter to utility
 * functions such as parse_http_request.
 *
 * @return an empty request
 */
http_request_t* http_request_create();

/**
 * Return the http request method or NULL. This function will never return NULL if the request was
 * parsed successfully.
 *
 * @param req: the request
 * @return the request method
 */
char* http_request_method(http_request_t *req);
void http_request_set_method(http_request_t *req, char *method);

/**
 * Return the http request uri or NULL. This function will never return NULL if the request was
 * parsed successfully.
 *
 * @param req: the request
 * @return the request uri
 */
char* http_request_uri(http_request_t *req);
void http_request_set_uri(http_request_t *req, char *uri);

/**
 * Return the http request version or NULL. This function will never return NULL if the request was
 * parsed successfully.
 *
 * @param req: the request
 * @return the request version ("HTTP/1.0", "HTTP/1.1", "HTTP/"2.0", etc)
 */
char* http_request_version(http_request_t *req);
void http_request_set_version(http_request_t *req, char *version);

/**
 * Return the http header with the given key or NULL if no such header was parsed.
 * 
 * @param req: the request
 * @param key: the header key
 * @return the header value or NULL
 */
http_headers_t* http_request_get_headers(http_request_t *req);

/**
 * Destroy the request and free all associated memory.
 * 
 * @param req: the request
 */
void http_request_destroy(http_request_t *req);

/**
 * Parse an http request from the buffer. Return the number of characters parsed from the buffer
 * or -1 if the request was unable to be parsed.
 *
 * @param buffer: the buffer
 * @param req: the http_request_t to fill
 * @return the number of characters successfully parsed or -1.
 */
long parse_http_request(buffer_t buffer, http_request_t *req);



http_response_t* http_response_create();
void http_response_destroy(http_response_t *res);

int http_response_get_status(http_response_t *res);
void http_response_set_status(http_response_t *res, int status);

http_headers_t* http_response_get_headers(http_response_t *res);

buffer_t* http_response_get_body(http_response_t *res);
void http_response_set_body(http_response_t *res, buffer_t body);

buffer_t http_response_write_head(http_response_t *res);

long parse_http_response(buffer_t buffer, http_response_t *res);

char* http_headers_get(http_headers_t *headers, char *key);
void http_headers_set(http_headers_t *headers, char *key, char *val);

#endif /* HTTP_H */

