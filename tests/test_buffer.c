#include <test.h>
#include <buffer.h>
#include <string.h>

void test_buffer_create() {
    buffer_t buffer = buffer_create(5);
    assert(buffer.length == 5);
    assert(buffer.data != NULL);
    uint8_t zero[5] = {0};
    assert(memcmp(buffer.data, zero, 5) == 0);
    buffer_destroy(buffer);
}

void test_buffer_create_from_string() {
    const char *str = "testingtesting123";
    buffer_t buffer1 = buffer_create_from_string(str);
    buffer_t buffer2 = buffer_create(strlen(str));
    memcpy(buffer2.data, str, strlen(str));
    assert(buffer1.length == buffer2.length);
    assert(memcmp(buffer1.data, buffer2.data, buffer1.length) == 0);
    buffer_destroy(buffer1);
    buffer_destroy(buffer2);
}

void test_buffer_copy() {
    const char *test_data = "testingtesting123";
    buffer_t buffer1 = buffer_create_from_string(test_data);
    buffer_t buffer2 = buffer_copy(buffer1);
    assert(memcmp(buffer1.data, buffer2.data, strlen(test_data)) == 0);
    buffer_destroy(buffer1);
    buffer_destroy(buffer2);
}

void test_buffer_compare() {
    buffer_t buffer1 = buffer_create_from_string("zz");
    buffer_t buffer2 = buffer_create_from_string("zzy");
    buffer_t buffer3 = buffer_create_from_string("zzz");

    assert(buffer_compare(buffer1, buffer1) == 0);
    assert(buffer_compare(buffer1, buffer2) < 0);
    assert(buffer_compare(buffer2, buffer3) < 0);

    buffer_destroy(buffer1);
    buffer_destroy(buffer2);
    buffer_destroy(buffer3);
}

void test_buffer_concat() {
    buffer_t buffer1 = buffer_create_from_string("foo");
    buffer_t buffer2 = buffer_create_from_string("bar");
    buffer_t buffer3 = buffer_concat(buffer1, buffer2);

    assert(buffer3.length == 6);
    assert(memcmp(buffer3.data, "foobar", 6) == 0);
    assert(buffer1.length == 3);
    assert(memcmp(buffer1.data, "foo", 3) == 0);
    assert(buffer2.length == 3);
    assert(memcmp(buffer2.data, "bar", 3) == 0);

    buffer_destroy(buffer1);
    buffer_destroy(buffer2);
    buffer_destroy(buffer3);
}


void test_buffer_append() {
    buffer_t buffer1 = buffer_create_from_string("foo");
    buffer_t buffer2 = buffer_create_from_string("bar");
    buffer_append(&buffer1, buffer2);

    assert(buffer1.length == 6);
    assert(memcmp(buffer1.data, "foobar", 6) == 0);
    assert(buffer2.length == 3);
    assert(memcmp(buffer2.data, "bar", 3) == 0);

    buffer_destroy(buffer1);
    buffer_destroy(buffer2);
}

void test_buffer_resize() {
    buffer_t buffer1 = buffer_create_from_string("foo");
    assert(buffer1.length == 3);
    buffer_resize(&buffer1, 6);
    assert(buffer1.length == 6);
    assert(memcmp(buffer1.data, "foo\0\0\0", 6) == 0);

    buffer_t buffer2 = buffer_create_from_string("bar");
    assert(buffer2.length == 3);
    buffer_resize(&buffer2, 2);
    assert(buffer2.length == 2);
    assert(memcmp(buffer2.data, "ba", 2) == 0);
    
    buffer_destroy(buffer1);
    buffer_destroy(buffer2);
}

int main(int argc, char *argv[]) {
    TEST(test_buffer_create)
    TEST(test_buffer_create_from_string)
    TEST(test_buffer_copy)
    TEST(test_buffer_compare)
    TEST(test_buffer_concat)
    TEST(test_buffer_append)
    TEST(test_buffer_resize)
}

