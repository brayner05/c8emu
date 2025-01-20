CC = gcc
CFLAGS = -g -Wall

SRC = src
BIN = bin

SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c,$(BIN)/%.o,$(SOURCES))
TARGET = $(BIN)/c8emu

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^

$(BIN)/%.o: $(SRC)/%.c
	mkdir -p -v bin
	$(CC) $(CFLAGS) -o $@ $< -c 

run:
	./bin/c8dp

clean:
	rm -rf bin

.PHONY: run
.PHONY: clean
