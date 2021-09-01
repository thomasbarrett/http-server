#include <path.h>
#include <ctype.h>

/**
 * Return 1 if the character `c` is unreserved, as defined in rfc3986. Unreserved characters
 * are always semantically identical to their percent encoded equivalent. Normalization
 * should convert all unreserved characters in percent encoded form to their char equivalent.
 */
int is_unreserved(char c) {
    return c == '-' || c == '.' || c == '_' || c == '~' || isalnum(c);
}

int is_gen_delim(char c) {
    switch (c) {
    case ':': case '/': case '?': case '#':
    case '[': case ']': case '@':
        return 1;
    default:
        return 0;
    }
}

int is_sub_delim(char c) {
    switch (c) {
    case '!': case '$': case '&': case '\'':
    case '(': case ')': case '*': case '+':
    case ',': case ';': case '=':
        return 1;
    default:
        return 0;
    }
}

static char hex_to_char(char c) {
    if (isdigit(c)) return c - '0';
    else if ('A' <= c && c <= 'F') return c - 'A' + 10;
    else if ('a' <= c && c <= 'f') return c - 'a' + 10;
    else return -1;
}

static char char_to_hex(char c) {
    if (c <= 9) return '0' + c;
    else return 'A' + c - 10;
}

/**
 * Return 1 is the character `c` is reserved, as defined in rfc3986. Reserved characters
 * may not be semantically identical to their percent encoded equivalent. Depending on the
 * URI scheme, normalization may or may not convert percent encoeded reserved characters to
 * their char equivalent.
 */
int is_reserved(char c) {
    return is_gen_delim(c) || is_sub_delim(c);
}

static long parse_percent_encoded(buffer_t buffer, char *c) {
    if (buffer.length < 3) return -1;
    if (buffer.data[0] != '%') return -2;
    char c1 = buffer.data[1];
    char c2 = buffer.data[2];
    if (!isxdigit(c1) || !isxdigit(c2)) return -2;
    char d1 = hex_to_char(c1);
    char d2 = hex_to_char(c2);
    *c = 16 * d1 + d2;
    return 3;
}

static long parse_pchar(buffer_t buffer, char *c) {
    if (buffer.length == 0) return -1;
    *c = buffer.data[0];
    if (is_unreserved(*c) || is_sub_delim(*c) || *c == ':' || *c == '@') {
        return 1;
    } else if (*c == '%') {
        return parse_percent_encoded(buffer, c);
    } else {
        return 0;
    }
}

long parse_absolute_path(buffer_view_t buf, array_t *segments) {
    if (buf.length == 0) return -1;
    if (buf.data[0] != '/') return -2;
    long acc = 0;
    while (buf.length > 0) {
        if (buf.data[0] != '/') break;
        buffer_slice(buf, 1, &buf);
        acc += 1;
        long len = parse_path_segment(buf);
        if (len < 0) return len;
        if (segments != NULL) {
            array_add(segments, &(buffer_view_t) {buf.data, len});
        }
        buffer_slice(buf, len, &buf);
        acc += len;
    }
    return acc;
}

long parse_path_segment(buffer_view_t buf) {
    long acc = 0;
    for (size_t i = 0; buf.length > 0; i++) {
        char c;
        long len = parse_pchar(buf, &c);
        if (len < 0) return len;
        else if (len == 0) break;
        else acc += len;
        buffer_slice(buf, len, &buf);
    }
    return acc;
}

int path_segment_encode(buffer_t buf, buffer_t *res) {
    if (buf.data == NULL) return -1;
    if (res == NULL) return -1;
    size_t n_encoded = 0;
    for (size_t i = 0; i < buf.length; i++) {
        if (is_reserved(buf.data[i])) n_encoded++;
    }
    *res = buffer_create(buf.length + 2 * n_encoded);
    size_t j = 0;
    for (size_t i = 0; i < buf.length; i++) {
        char c = buf.data[i];
        if (is_reserved(c)) {
            res->data[j] = '%';
            res->data[j + 1] = char_to_hex(c >> 4);
            res->data[j + 2] = char_to_hex(c & 0x0f);
            j += 3;
        } else {
            res->data[j] = c;
            j += 1;
        }
    }
    return 0;
}

int path_segment_decode(buffer_t buf, buffer_t *res) {
    if (buf.data == NULL) return -1;
    if (res == NULL) return -1;
    size_t n_encoded = 0;
    for (size_t i = 0; i < buf.length; i++) {
        if (buf.data[i] == '%') n_encoded++;
    }
    *res = buffer_create(buf.length - 2 * n_encoded);
    for (size_t i = 0; buf.length > 0; i++) {
        char c;
        long len = parse_pchar(buf, &c);
        if (len <= 0) return -1;
        buffer_slice(buf, len, &buf);
        res->data[i] = c;
    }
    return 0;
}

int path_normalize(buffer_t path, buffer_t *res) {
    return -1;
}

