#include <test.h>
#include <path.h>
#include <string.h>
#include <stdio.h>

void test_path_segment_encode_error() {
    int res = 0;
    buffer_t buf = {0};

    char *str = "hello";
    res = path_segment_encode((buffer_t){(uint8_t*) str, strlen(str)}, NULL);
    assert(res == -1);

    res = path_segment_encode((buffer_t){NULL, 0}, &buf);
    assert(res == -1);

    res = path_segment_encode((buffer_t){NULL, 23}, &buf);
    assert(res == -1);
}

void test_path_segment_encode_helper(char *in, char *out) {
    int res = 0;
    buffer_t buf = {0};
    res = path_segment_encode((buffer_t){(uint8_t*) in, strlen(in)}, &buf);
    assert(res == 0);
    assert(buf.length == strlen(out));
     if (strcmp((char *) buf.data, out) != 0) {
        printf("expected:'%s'\nactually: '%.*s'\n", out, (int) buf.length, buf.data);
        assert(0 && "fail");
    }
    buffer_destroy(buf);
}

void test_path_segment_encode() {
    test_path_segment_encode_helper("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");
    test_path_segment_encode_helper("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    test_path_segment_encode_helper("0123456789", "0123456789");
    test_path_segment_encode_helper("-_.~", "-_.~");
    test_path_segment_encode_helper(":/?#[]@", "%3A%2F%3F%23%5B%5D%40");
    test_path_segment_encode_helper("!$&'()*+,;=", "%21%24%26%27%28%29%2A%2B%2C%3B%3D");
}

void test_path_segment_decode_helper(char *in, char *out) {
    int res = 0;
    buffer_t buf = {0};
    res = path_segment_decode((buffer_t){(uint8_t*) in, strlen(in)}, &buf);
    assert(res == 0);
    assert(buf.length == strlen(out));
    if (strcmp((char *) buf.data, out) != 0) {
        printf("expected '%s':\nactually: '%.*s'", out, (int) buf.length, buf.data);
        assert(0 && "fail");
    }
    buffer_destroy(buf);
}

void test_path_segment_decode() {
    test_path_segment_decode_helper("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");
    test_path_segment_decode_helper("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    test_path_segment_decode_helper("0123456789", "0123456789");
    test_path_segment_decode_helper("-_.~", "-_.~");
    test_path_segment_decode_helper("%3A%2F%3F%23%5B%5D%40", ":/?#[]@");
    test_path_segment_decode_helper("%21%24%26%27%28%29%2A%2B%2C%3B%3D", "!$&'()*+,;=");
}

void test_parse_absolute_path() {
    buffer_t buf = buffer_create_from_string("/foo/bar/baz");
    long res = parse_absolute_path(buf, NULL);
    assert(res == buf.length);
    buffer_destroy(buf);

    buffer_t buf2 = buffer_create_from_string("foo/bar/baz");
    long res2 = parse_absolute_path(buf2, NULL);
    assert(res2 == -2);
    buffer_destroy(buf2);

    buffer_t buf3 = buffer_create_from_string("");
    long res3 = parse_absolute_path(buf3, NULL);
    assert(res3 == -1);
    buffer_destroy(buf3);
}


void test_parse_absolute_path_with_segments() {
    array_t *segments = array_create(sizeof(buffer_view_t), 3);
    buffer_t buf = buffer_create_from_string("/foo/bar/baz");
    long res = parse_absolute_path(buf, segments);
    assert(res == buf.length);
    assert(array_size(segments) == 3);
    buffer_view_t *seg0 = array_get(segments, 0);
    assert(seg0->length == 3);
    assert(memcmp("foo", (char *) seg0->data, 3) == 0);
    buffer_view_t *seg1 = array_get(segments, 1);
     assert(seg1->length == 3);
    assert(memcmp("bar", (char *) seg1->data, 3) == 0);
    buffer_view_t *seg2 = array_get(segments, 2);
    assert(seg2->length == 3);
    assert(memcmp("baz", (char *) seg2->data, 3) == 0);
    buffer_destroy(buf);
    array_destroy(segments, NULL);
}

int main(int argc, char *argv[]) {
    TEST(test_path_segment_encode_error);
    TEST(test_path_segment_encode);
    TEST(test_path_segment_decode);
    TEST(test_parse_absolute_path);
    TEST(test_parse_absolute_path_with_segments);
}
