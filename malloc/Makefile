CC = gcc
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -Wall -Wextra -Werror -std=c99 -Wvla
LDFLAGS = -shared
VPATH = src

TARGET_LIB = libmalloc.so
OBJS = malloc.o
BIN = main

all: library

library: $(TARGET_LIB)
$(TARGET_LIB): CFLAGS += -pedantic -fvisibility=hidden -fPIC
$(TARGET_LIB): LDFLAGS += -Wl,--no-undefined
$(TARGET_LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

debug: CFLAGS += -g
debug: clean $(TARGET_LIB)

check: CFLAGS += -g -DDEBUG -o $(BIN)
check:
	$(CC) $(CFLAGS) $(CPPFLAGS) src/malloc.c tests/main.c
	./main

clean:
	$(RM) $(TARGET_LIB) $(OBJS) $(BIN)

.PHONY: all $(TARGET_LIB) clean
