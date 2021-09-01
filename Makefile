CC = clang

CFLAGS = -std=c11 -fsanitize=address -O0 -g -Iinclude -I/usr/local/include -L/usr/local/lib -Wall -pedantic -Wno-unused-command-line-argument -pthread
SRC_FILES = buffer list array log tree_map path tcp http tcp_socket
OBJ_FILES = $(addprefix obj/,$(SRC_FILES:=.o))

MAIN = main http client
MAIN_BINS = $(addprefix bin/,$(MAIN))
TEST_BINS = $(addprefix bin/test_,$(SRC_FILES))
LIBS = 

all: $(MAIN_BINS) $(TEST_BINS)

obj:
	mkdir obj

	mkdir bin

bin/%: main/%.c $(OBJ_FILES) | bin
	$(CC) $(CFLAGS) $^ -o $@

obj/%.o: src/%.c | obj
	$(CC) -c $(CFLAGS) $^ -o $@
obj/%.o: main/%.c | obj
	$(CC) -c $(CFLAGS) $^ -o $@
obj/%.o: tests/%.c | obj
	$(CC) -c $(CFLAGS) $^ -o $@
	
bin/test_%: tests/test_%.c $(OBJ_FILES) | bin
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf bin
	rm -rf obj

test: $(TEST_BINS)
	@for f in $(TEST_BINS); do echo $$f; ASAN_OPTIONS=detect_leaks=1 $$f; echo; done

memcheck:
	ASAN_OPTIONS=detect_leaks=1 ./bin/main

.PHONY: all clean
