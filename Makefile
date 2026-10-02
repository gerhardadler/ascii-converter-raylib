PLATFORM ?= PLATFORM_DESKTOP
CC      := gcc
BIN     := build/ascii_converter
WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wconversion
CFLAGS  := -std=c11 $(WARN) -MMD -MP
CFLAGS  += -Iinclude -isystem vendor/raygui -isystem vendor/stb_truetype -isystem vendor/parse_args
CFLAGS  += $(shell pkg-config --cflags raylib)
LDLIBS  := $(shell pkg-config --libs raylib) -lm

# Debug flags by default
CFLAGS  += -g -O0

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/raygui_impl.o: CFLAGS := $(filter-out $(WARN) -g -O0,$(CFLAGS)) -w -O3 -DNDEBUG
build/stb_truetype.o: CFLAGS := $(filter-out $(WARN) -g -O0,$(CFLAGS)) -w -O3 -DNDEBUG

build:
	mkdir -p build

run: $(BIN)
	./$(BIN) $(ARGS)

release: CFLAGS := $(filter-out -g -O0,$(CFLAGS)) -O3 -DNDEBUG
release: clean $(BIN)

clean:
	rm -rf build

-include $(DEP)
.PHONY: all run clean release
