CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -O2
SRC := $(shell find src -name "*.c")
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
TARGET := prox1
LIBS := -lX11 -lGL -lXrandr -lm

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Isrc -Iext -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: clean
