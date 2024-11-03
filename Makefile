# Variables
CC = gcc
CFLAGS = -fPIC -Wall -Iinc
LDFLAGS = -shared
TARGET_LIB = bin/libmylibrary.so
LIB_SRC = $(wildcard src/*.c)
EXM_SRC = $(wildcard exm/*.c)
BIN = bin
EXAMPLE_SRC = exm/example.c
EXAMPLE_EXEC = bin/example


build: $(LIB_SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN)/libmylibrary.so $^

build-example: build $(EXM_SRC)
	$(CC) $(CFLAGS) -o $(BIN)/example $(EXM_SRC) -L$(BIN) -lmylibrary -Wl,-rpath=.
# Rules
all: bin $(TARGET_LIB)

# Create bin directory if it doesn't exist
bin:
	mkdir -p bin

# Compile shared library
$(TARGET_LIB): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Compile example program, linking against the shared library
example: $(TARGET_LIB) $(EXAMPLE_SRC)
	$(CC) -Wall -o $(EXAMPLE_EXEC) $(EXAMPLE_SRC) -Iinc -Lbin -lmylibrary

# Clean up build artifacts
clean:
	rm -rf bin/*.so bin/example

.PHONY: all clean example
