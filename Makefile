CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -O2 -MMD -MP
SRC := $(shell find src -name "*.c")
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
TARGET := prox1
LIBS := -lX11 -lGL -lXrandr -lm

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Isrc -Iext -c $< -o $@

# Unit tests need no window or GPU
build/test_config: tests/test_config.c src/config.c src/vector_field.c
	@mkdir -p build
	$(CC) -std=c99 -Wall -Wextra -Isrc $^ -o $@

test: build/test_config
	./build/test_config

clean:
	rm -rf build $(TARGET)

-include $(OBJ:.o=.d)

.PHONY: clean test
