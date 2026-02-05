CXX := g++
CC  := clang

SRC_DIR   := ./source
BUILD_DIR := ./build
TARGET    := main
SOURCES   := $(SRC_DIR)/main.c

INCLUDES := -I$(SRC_DIR)

CFLAGS := -g -ggdb

LDFLAGS := -I. -lGLX  -lX11 -lGL -lGLX

.PHONY: all clean run

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	$(Q)mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR) $(SOURCES)
	$(Q)$(CC) $(INCLUDES) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	$(Q)rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET)
	$(Q)./$<
