PLATFORM ?= PLATFORM_DESKTOP

CC      := gcc
BIN     := build/ascii_converter

WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wconversion
CFLAGS  := -std=c11 -g -O3 -fopenmp $(WARN) -MMD -MP
CFLAGS  += -Iinclude -isystem vendor/raygui -isystem vendor/stb_truetype -isystem vendor/parse_args
CFLAGS  += $(shell pkg-config --cflags raylib)
LDLIBS  := $(shell pkg-config --libs raylib) -lm -fopenmp

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# raygui's implementation is not our code — compile it silently.
build/raygui_impl.o: CFLAGS := $(filter-out $(WARN),$(CFLAGS)) -w

# stb_truetype's implementation is not our code — compile it silently.
build/stb_truetype.o: CFLAGS := $(filter-out $(WARN),$(CFLAGS)) -w

build:
	mkdir -p build

run: $(BIN)
	./$(BIN) $(ARGS)

clean:
	rm -rf build

-include $(DEP)

.PHONY: all run clean
