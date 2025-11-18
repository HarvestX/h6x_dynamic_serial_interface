CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
PROJECT = h6x_dynamic_packet_handler
VERSION = 0.0.1

SRC_DIR = ../../src
INC_DIR = ../../include
BUILD_DIR = build
LIB_DIR = lib/arm-none-eabi
INSTALL_PREFIX = /usr/local

STM32F4 ?= 0

ifeq ($(STM32F4),1)
CFLAGS = -mcpu=cortex-m4 \
         -mthumb \
         -mfloat-abi=hard \
         -mfpu=fpv4-sp-d16 \
         -Wall \
         -Wextra \
         -Wpedantic \
         -Os \
         -ffunction-sections \
         -fdata-sections \
         -I$(INC_DIR) \
         -specs=nosys.specs \
         -nostartfiles \
         -nodefaultlibs \
         -nostdlib
else
CFLAGS = -mcpu=cortex-m0plus \
         -mthumb \
         -mfloat-abi=soft \
         -Wall \
         -Wextra \
         -Wpedantic \
         -Os \
         -ffunction-sections \
         -fdata-sections \
         -I$(INC_DIR) \
         -specs=nosys.specs \
         -nostartfiles \
         -nodefaultlibs \
         -nostdlib
endif

DEBUG_CFLAGS = -g -O0

RELEASE_CFLAGS = -O2 -DNDEBUG

SOURCES = $(SRC_DIR)/h6x_dynamic_packet_crc8.c \
          $(SRC_DIR)/h6x_dynamic_packet_definitions_base.c \
          $(SRC_DIR)/h6x_dynamic_packet_handler.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

LIB_TARGET = $(BUILD_DIR)/lib$(PROJECT).a

all: $(LIB_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LIB_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_TARGET): $(OBJECTS)
	$(AR) rcs $@ $^
	$(SIZE) $@

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(LIB_TARGET)

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(LIB_TARGET)

stm32f4: STM32F4=1
stm32f4: $(LIB_TARGET)

stm32f4-debug: STM32F4=1
stm32f4-debug: CFLAGS += $(DEBUG_CFLAGS)
stm32f4-debug: $(LIB_TARGET)

stm32f4-release: STM32F4=1
stm32f4-release: CFLAGS += $(RELEASE_CFLAGS)
stm32f4-release: $(LIB_TARGET)

clean:
	rm -rf $(BUILD_DIR)

install: $(LIB_TARGET)
	mkdir -p $(INSTALL_PREFIX)/$(LIB_DIR)
	mkdir -p $(INSTALL_PREFIX)/include/h6x_dynamic_packet_handler
	cp $(LIB_TARGET) $(INSTALL_PREFIX)/$(LIB_DIR)/
	cp -r $(INC_DIR)/h6x_dynamic_packet_handler/*.h $(INSTALL_PREFIX)/include/h6x_dynamic_packet_handler/

install-local: $(LIB_TARGET)
	mkdir -p $(LIB_DIR)
	cp $(LIB_TARGET) $(LIB_DIR)/

uninstall:
	rm -f $(INSTALL_PREFIX)/$(LIB_DIR)/lib$(PROJECT).a
	rm -rf $(INSTALL_PREFIX)/include/h6x_dynamic_packet_handler

info:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "LIB_TARGET: $(LIB_TARGET)"

.PHONY: all debug release stm32f4 stm32f4-debug stm32f4-release clean install install-local uninstall info