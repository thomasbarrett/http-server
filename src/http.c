#include <http.h>
#include <buffer.h>
#include <array.h>

#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <path.h>

typedef struct http_request {
    char* method;
    char* uri;
    char* version;
    array_t *headers;
} http_request_t;

typedef struct http_response {
    char *version;
    int status;
    array_t *headers;
    buffer_t body;
} http_response_t;

typedef struct http_header {
    char* key;
    char* value;
} http_header_t;

static void http_header_destroy(http_header_t *header) {
    if (header == NULL) return;
    free(header->key);
    free(header->value);
}


static char* string_copy(char *a) {
    size_t n = strlen(a);
    char *b = calloc(1, n + 1);
    assert(b != NULL && "out of memory");
    memcpy(b, a, n);
    return b;
}


http_request_t* http_request_create() {
    http_request_t *res = calloc(1, sizeof(http_request_t));
    assert(res != NULL && "out of memory");
    res->headers = array_create(sizeof(http_header_t), 16);
    return res;
}

char* http_request_method(http_request_t *req) {
    return req->method;
}

void http_request_set_method(http_request_t *req, char *method) {
    req->method = string_copy(method);
}

char* http_request_uri(http_request_t *req) {
    return req->uri;
}

void http_request_set_uri(http_request_t *req, char *uri) {
    req->uri = string_copy(uri);
}

char* http_request_version(http_request_t *req) {
    return req->version;
}

void http_request_set_version(http_request_t *req, char *version) {
    req->version = string_copy(version);
}

array_t* http_request_get_headers(http_request_t *req) {
    return req->headers;
}

char* http_request_header(http_request_t *req, char *key) {
    size_t n = strlen(key);
    char lowercase_key[n + 1];
    for (size_t i = 0; i <= n; i++) {
        lowercase_key[i] = tolower(key[i]);
    }
    for (size_t i = 0; i < array_size(req->headers); i++) {
        http_header_t *header = array_get(req->headers, i);
        if (strcmp(header->key, lowercase_key) == 0) return header->value; 
    }
    return NULL;
}

void http_request_destroy(http_request_t *req) {
    if (req == NULL) return;
    array_destroy(req->headers, (destroy_t) http_header_destroy);
    free(req->method);
    free(req->uri);
    free(req->version);
}

static int is_ascii(uint8_t c) {
    return c <= 127;
}

static int is_vchar(uint8_t c) {
    return 0x21 <= c && c <= 0x7E;
}

static int is_space(uint8_t c) {
    return c == ' ' || c == '\t';
}

static int is_obs_text(uint8_t c) {
    return 0x80 <= c && c <= 0xFF;
}

static int is_tchar(uint8_t c) {
    switch (c) {
    case '!': case '#': case '$': case '%':
    case '&': case '*': case '+': case '-':
    case '.': case '^': case '_': case '`':
    case '|': case '~': case '\'':
        return 1;
    }
    return isdigit(c) || isalpha(c);
}

static long parse_http_token(buffer_t buffer, buffer_t *res) {
    if (buffer.length == 0) return -1;
    if (!is_tchar(buffer.data[0])) return -2;
    for (size_t i = 0; i < buffer.length; i++) {
        if (!is_tchar(buffer.data[i])) {
            res->data = buffer.data;
            res->length = i;
            return i;
        }
    }
    res->data = buffer.data;
    res->length = buffer.length;
    return buffer.length;
}

static long parse_http_space(buffer_t buffer) {
    if (buffer.length == 0) return -1;
    if (!is_space(buffer.data[0])) return -2;
    for (size_t i = 0; i < buffer.length; i++) {
        if (!is_space(buffer.data[i])) {
            return i;
        }
    }
    return buffer.length;
}

static long parse_http_version(buffer_t buffer, buffer_t *res) {
    if (buffer.length < 5) {
        if (memcmp(buffer.data, "HTTP/", buffer.length) == 0) return -1;
        else return -2;
    }
    if (memcmp(buffer.data, "HTTP/", 5) != 0) return -2;
    if (buffer.length < 6) return -1;
    if (!isdigit(buffer.data[5])) return -2;
    if (buffer.length < 7) return -1;
    if (buffer.data[6] != '.') return -2;
    if (buffer.length < 8) return -1;
    if (!isdigit(buffer.data[7])) return -2;
    res->data = buffer.data;
    res->length = 8;
    return 8;
}

static long parse_http_newline(buffer_t buffer) {
    if (buffer.length < 1) return -1;
    if (buffer.data[0] != '\r') return -2;
    if (buffer.length < 2) return -1;
    if (buffer.data[1] != '\n') return -2;
    return 2;
}

static long parse_http_request_line(buffer_t buffer, buffer_t *method, buffer_t *uri, buffer_t *version) {
    long len, acc_len = 0;

    len = parse_http_token(buffer, method);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    if (buffer.length < 1) return -1;
    if (buffer.data[0] != ' ') return -2;
    acc_len += 1;
    buffer_slice(buffer, 1, &buffer);

    len = parse_absolute_path(buffer, NULL);
    if (len < 0) return len;
    acc_len += len;
    *uri = (buffer_view_t) {buffer.data, len};
    buffer_slice(buffer, len, &buffer);

    if (buffer.length < 1) return -1;
    if (buffer.data[0] != ' ') return -2;
    acc_len += 1;
    buffer_slice(buffer, 1, &buffer);

    len = parse_http_version(buffer, version);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    len = parse_http_newline(buffer);
    if (len < 0) return len;
    acc_len += len;

    return acc_len; 
}

static long parse_http_status_code(buffer_t buffer, int *status) {
    *status = 0;
    if (buffer.length < 3) return -1;
    if (!isdigit(buffer.data[0])) return -2;
    if (!isdigit(buffer.data[1])) return -2;
    if (!isdigit(buffer.data[2])) return -2;
    *status += (buffer.data[0] - '0') * 100;
    *status += (buffer.data[1] - '0') * 10;
    *status += (buffer.data[2] - '0') * 1;
    return 3;
}

static long parse_http_reason_phrase(buffer_t buffer) {
    for (size_t i = 0; i < buffer.length; i++) {
        char c = buffer.data[i];
        if (c != ' ' && c != '\t' && !is_vchar(c) && !is_obs_text(c)) {
            return i;
        }
    }
    return buffer.length;
}

static long parse_http_status_line(buffer_t buffer, buffer_t *version, int *status) {
    long len, acc_len = 0;

    len = parse_http_version(buffer, version);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    if (buffer.length < 1) return -1;
    if (buffer.data[0] != ' ') return -2;
    acc_len += 1;
    buffer_slice(buffer, 1, &buffer);

    len = parse_http_status_code(buffer, status);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    if (buffer.length < 1) return -1;
    if (buffer.data[0] != ' ') return -2;
    acc_len += 1;
    buffer_slice(buffer, 1, &buffer);

    len = parse_http_reason_phrase(buffer);
    if (len < 0) return len;
    acc_len += len;

    return acc_len;
}

/**
 * parse_http_header_value parses part of a header field as defined in rfc7230. 
 * 
 * header-field = field-name ":" OWS field-value OWS
 * field-value = *(field-content / obs-fold)
 * field-content = field-vchar [ 1*(SP / HTAB) field-vchar ]
 * field-vchar = VCHAR / obs-text
 * obs-fold = CRLF 1*(SP / HTAB)
 * *(SP / HTAB / VCHAR / obs-text)
 *
 */
static long parse_http_header_value(buffer_t buffer, buffer_t *value) {
    size_t length = 0;
    for (size_t i = 0; i < buffer.length; i++) {
        char c = buffer.data[i];
        if (!is_vchar(c) && !is_space(c) && !is_obs_text(c)) {
            length = i;
            break;
        }
    }
    if (buffer.length - length < 2) return -1;
    if (buffer.data[length + 0] != '\r') return -2;
    if (buffer.data[length + 1] != '\n') return -2;
    value->length = length;
    value->data = buffer.data;
    return length + 2;
}

static long parse_http_header_line(buffer_t buffer, buffer_t *key, buffer_t *value) {
    long len, acc_len = 0;

    len = parse_http_token(buffer, key);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    if (buffer.length < 1) return -1;
    if (buffer.data[0] != ':') return -2;
    acc_len += 1;
    buffer_slice(buffer, 1, &buffer);

    len = parse_http_header_value(buffer, value);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    return acc_len;
}

static buffer_t buffer_strip(buffer_t buffer) {
    size_t start = buffer.length;
    for (size_t i = 0; i < buffer.length; i++) {
        if (!is_space(buffer.data[i])) {
            start = i;
            break;
        }
    }
    size_t end = start;
    for (size_t i = buffer.length - 1; i >= start; i--) {
        if (!is_space(buffer.data[i])) {
            end = i + 1;
            break;
        }
    }
    return (buffer_t){buffer.data + start, end - start};
}

static void string_to_lower(char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

long parse_http_headers(buffer_t buffer, array_t *headers) {
    long len, acc_len = 0;

    // attempt to parse headers until empty line or error 
    while (1) {
        http_header_t header;
        buffer_t name, value;
        len = parse_http_header_line(buffer, &name, &value); 
        if (len == -1) return -1;
        if (len == -2) {
            len = parse_http_newline(buffer);
            if (len < 0) return len;
            acc_len += len;
            return acc_len;
        }
        acc_len += len;
        buffer_slice(buffer, len, &buffer);
        
        header.key = buffer_to_string(name);
        string_to_lower(header.key);
        header.value = buffer_to_string(buffer_strip(value));
    }
}

long parse_http_request(buffer_t buffer, http_request_t *req) {
    long len, acc_len = 0; 

    buffer_t method, uri, version;

    len = parse_http_request_line(buffer, &method, &uri, &version);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    req->method = buffer_to_string(method);
    req->uri = buffer_to_string(uri);
    req->version = buffer_to_string(version);
    req->headers = array_create(sizeof(http_header_t), 16);

    len = parse_http_headers(buffer, req->headers);
    if (len < 0) return len;
    acc_len += len;
    return acc_len;
}

http_response_t* http_response_create() {
    http_response_t *res = calloc(1, sizeof(http_response_t));
    assert(res != NULL && "out of memory");
    char *version = "HTTP/1.1";
    res->version = calloc(1, strlen(version) + 1); 
    strcpy(res->version, version);
    res->status = 200;
    res->headers = array_create(sizeof(http_header_t), 16);
    return res;
}


void http_response_destroy(http_response_t *res) {
    if (res == NULL) return;
    free(res->version);
    array_destroy(res->headers, (destroy_t) http_header_destroy);
    buffer_destroy(res->body);
}

static int key_equal(char *a, char *b) {
    for (size_t i = 0; a[i] != '\0'; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

int http_response_get_status(http_response_t *res) {
    return res->status;
}

void http_response_set_status(http_response_t *res, int status) {
    res->status = status;
}

array_t* http_response_get_headers(http_response_t *res) {
    return res->headers;
}

char* http_headers_get(array_t *headers, char *key) {
    for (size_t i = 0; i < array_size(headers); i++) {
        http_header_t *header = array_get(headers, i);
        if (key_equal(header->key, key) == 1) return header->value;
    }
    return NULL;
}

static char* string_join(char *a, char *b) {
    size_t m = strlen(a);
    size_t n = strlen(b);
    char *c = realloc(a, m + n + 2);
    assert(c != NULL && "out of memory");
    c[m] = ',';
    memcpy(c + m + 1, b, n);
    return c;
}

void http_headers_set(array_t *headers, char *key, char *val) {
    
    /* if key already exists: join values with comma */
    for (size_t i = 0; i < array_size(headers); i++) {
        http_header_t *header = array_get(headers, i);
        if (key_equal(header->key, key) == 1) {
            header->value = string_join(header->value, val);
        }
    }

    /* if key does not exist: create new header */
    http_header_t header;
    header.key = string_copy(key);
    header.value = string_copy(val);
    array_add(headers, &header);

}

buffer_t* http_response_get_body(http_response_t *res) {
    return &res->body;
}

void http_response_set_body(http_response_t *res, buffer_t body) {
    if (res->body.data != NULL) buffer_destroy(res->body);
    res->body = body;
}

static char* status_to_string(int status) {
    switch (status) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 426: return "Upgrade Required";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Time-out";
    case 505: return "HTTP Version not supported";
    default: return "Unknown";
    }
}

buffer_t http_response_write_head(http_response_t *res) {
    char *reason_phrase = status_to_string(res->status); 
    size_t len = 0, head_len = 0;

    /* compute response head length */
    head_len += strlen("HTTP/1.1 200 ");
    head_len += strlen(reason_phrase);
    head_len += strlen("\r\n");
    for (size_t i = 0; i < array_size(res->headers); i++) {
        http_header_t *header = array_get(res->headers, i);
        head_len += strlen(header->key);
        head_len += strlen(": ");
        head_len += strlen(header->value);
        head_len += strlen("\r\n");
    }
    head_len += strlen("\r\n");
    
    /* write response head */
    buffer_t buf = buffer_create(head_len);
    len += sprintf((char*) buf.data, "%8s %03d %s\r\n", res->version, res->status, reason_phrase);
    for (size_t i = 0; i < array_size(res->headers); i++) {
        http_header_t *header = array_get(res->headers, i);
        len += sprintf((char*) buf.data + len, "%s: %s\r\n", header->key, header->value); 
    }
    buf.data[len++] = '\r';
    buf.data[len++] = '\n';
    return buf; 
}


long parse_http_response(buffer_t buffer, http_response_t *res) {
    long len, acc_len = 0; 

    buffer_t version;
    int status;

    len = parse_http_status_line(buffer, &version, &status);
    if (len < 0) return len;
    acc_len += len;
    buffer_slice(buffer, len, &buffer);

    res->version = buffer_to_string(version);
    res->status = status;

    len = parse_http_headers(buffer, res->headers);
    if (len < 0) return len;
    acc_len += len;
    return acc_len;
}
