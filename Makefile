CC := gcc
DBG_BIN := lldb
#CFLAGS = -D_GNU_SOURCE
CFLAGS += -std=c11
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic
# CFLAGS += -Werror
CFLAGS += -Wmissing-declarations
CFLAGS += -I./libs/
ASANFLAGS=-fsanitize=address -fno-common -fno-omit-frame-pointer
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS := $(shell pkg-config --libs sdl2)
LIBS :=
SRC_FILES := $(filter-out ./src/TILES.c ./src/LEVEL.c, $(wildcard ./src/*.c))
BIN_DIR := ./bin
BIN := $(BIN_DIR)/hh
RES_DIR := ./res

build: bin-dir
	$(CC) $(CFLAGS) $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

bin-dir:
	@mkdir -p $(BIN_DIR)

res-dir:
	@mkdir -p $(RES_DIR)

debug: debug-build
	$(DBG_BIN) $(BIN) $(ARGS)

debug-build: bin-dir
	$(CC) $(CFLAGS) -g $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

extract-tiles: bin-dir res-dir
	$(CC) $(CFLAGS) $(LIBS) ./src/tiles.c -o ./bin/tx $(LDFLAGS)
	./bin/tx

extract-levels: bin-dir res-dir
	$(CC) $(CFLAGS) $(LIBS) ./src/level.c -o ./bin/tx $(LDFLAGS)
	./bin/tx

run: build
	@$(BIN) $(ARGS)

memcheck:
	@$(CC) -g $(SRC) $(ASANFLAGS) $(CFLAGS) $(INCS) $(LIBS) $(LFLAGS) -o memcheck.out
	@./memcheck.out
	@echo "Memory check passed"

clean:
	rm -rf $(BIN_DIR)/*

gen-compilation-db:
	bear -- make build

gen-compilation-db-make:
	make --always-make --dry-run \
	| grep -wE 'gcc|g\+\+' \
	| grep -w '\-c' \
	| jq -nR '[inputs|{directory:".", command:., file: match(" [^ ]+$").string[1:]}]' \
	> compile_commands.json
