# Makefile for h6x_dynamic_packet_handler x86_64 Linux build

# Compiler and tools
CC = gcc
AR = ar
OBJCOPY = objcopy
SIZE = size
PROJECT = h6x_dynamic_packet_handler
VERSION = 1.0.0

SRC_DIR = ../../src
INC_DIR = ../../include
BUILD_DIR = build
LIB_DIR = lib/x86_64-linux-gnu
INSTALL_PREFIX = /usr/local

# Compiler flags for x86_64 Linux
CFLAGS = -Wall \
         -Wextra \
         -Wpedantic \
         -fPIC \
         -I$(INC_DIR)

DEBUG_CFLAGS = -g -O0

RELEASE_CFLAGS = -O2 -DNDEBUG

# Linker flags for shared library
LDFLAGS = -shared -Wl,-soname,lib$(PROJECT).so.1

SOURCES = $(SRC_DIR)/h6x_dynamic_packet_crc8.c \
          $(SRC_DIR)/h6x_dynamic_packet_definitions_base.c \
          $(SRC_DIR)/h6x_dynamic_packet_handler.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

SHARED_LIB = $(BUILD_DIR)/lib$(PROJECT).so.$(VERSION)
SHARED_LIB_SONAME = $(BUILD_DIR)/lib$(PROJECT).so.1
SHARED_LIB_LINK = $(BUILD_DIR)/lib$(PROJECT).so

all: $(SHARED_LIB)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LIB_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create shared library
$(SHARED_LIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	cd $(BUILD_DIR) && ln -sf lib$(PROJECT).so.$(VERSION) lib$(PROJECT).so.1
	cd $(BUILD_DIR) && ln -sf lib$(PROJECT).so.1 lib$(PROJECT).so
	$(SIZE) $@

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(SHARED_LIB)

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(SHARED_LIB)

clean:
	rm -rf $(BUILD_DIR)
 and headers
install: $(SHARED_LIB)
	mkdir -p $(INSTALL_PREFIX)/$(LIB_DIR)
	mkdir -p $(INSTALL_PREFIX)/include/h6x_dynamic_packet_handler
	cp $(SHARED_LIB) $(INSTALL_PREFIX)/$(LIB_DIR)/
	cp $(SHARED_LIB_SONAME) $(INSTALL_PREFIX)/$(LIB_DIR)/
	cp $(SHARED_LIB_LINK) $(INSTALL_PREFIX)/$(LIB_DIR)/
	cp -r $(INC_DIR)/h6x_dynamic_packet_handler/*.h $(INSTALL_PREFIX)/include/h6x_dynamic_packet_handler/

install-local: $(SHARED_LIB)
	mkdir -p $(LIB_DIR)
	cp $(SHARED_LIB) $(LIB_DIR)/
	cp $(SHARED_LIB_SONAME) $(LIB_DIR)/
	cp $(SHARED_LIB_LINK) $(LIB_DIR)/

uninstall:
	rm -f $(INSTALL_PREFIX)/$(LIB_DIR)/lib$(PROJECT).so*
	rm -rf $(INSTALL_PREFIX)/include/h6x_dynamic_packet_handler

info:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "SHARED_LIB: $(SHARED_LIB)"

.PHONY: all debug release clean install install-local uninstall info