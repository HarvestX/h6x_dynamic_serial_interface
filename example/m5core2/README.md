
# How to Add a New Command

This document explains how to add a new command. The content of the command to be added is as follows:

```
Command Number: 250
Return Value: ASCII representation of a randomly generated string "rnd_" + 8-digit number (e.g., "rnd_48314168")
```

## primary

### Add to the M5Stack Display Array

In `example\m5core2\primary\main.cpp`, add the string you want to display in the M5Stack selection screen:

```cpp
const char* options[] = {
  "00: Ping", "01: Change print color (red/green)", "02: Reboot device",
  "03: Request general status", "03: Request firmware version",
  "10: Request device tick", "12: Request firmware write date",
  "13: Request device vendor", "14: Request device name",
  "15: Request current state", "20: Display CPU usage",
  "250: Request output random number" // NEW
};
```

Similarly, add the new command number to the following array. Use hexadecimal notation, so write it as `0xFA`.

```cpp
const uint8_t return_values[] = {
  0x00, 0x01, 0x02, 0x03, 0x03, 0x10, 0x12, 0x13, 0x14, 0x15, 0x20, 0xFA // NEW
};
```

### Add Command Constant Definition

Add the new command constant in `example\m5core2\include\protocol_definitions.h`:

```cpp
// === CUSTOM COMMAND DEFINITIONS ===
#define CMD_REQUEST_IMU                    0x20
#define CMD_REQUEST_CARRIPLATION_STATUS    0x21
#define CMD_REQUEST_CARRIPLATION_EXECUSION 0x96
#define CMD_REQUEST_OUTPUT_RANDOM_NUMBER   0xFA // NEW
```

### Add Processing Logic

Add the command processing logic in `example\m5core2\include\command_handler.hpp` under the `command_handler()` function.

This command generates a string like `"rnd_(random 8-digit number)"`, converts it to ASCII, and sends it via serial communication:

```cpp
case CMD_REQUEST_OUTPUT_RANDOM_NUMBER: {
    uint32_t rand_number = random(10000000, 100000000);

    char message[13];
    snprintf(message, sizeof(message), "rnd_%08lu", (unsigned long)rand_number);

    uint8_t send_data[16];
    uint8_t send_data_len = 0;
    convert_date_to_ascii_array(message, send_data, &send_data_len, sizeof(send_data));

    pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, ERR_SUCCESS, mode);
    break;
  }
```

### Modify `serial_primary.py`

Define the command constant on the PC side:

```python
# === COMMAND DEFINITIONS ===
CMD_PING                       = 0x00
CMD_INTERNAL_LED_ON_OFF       = 0x01
CMD_REBOOT_DEVICE             = 0x02
CMD_REQUEST_GENERAL_STATUS    = 0x03
CMD_REQUEST_FIRMWARE_VERSION  = 0x03
CMD_REQUEST_DEVICE_TICK       = 0x10
CMD_REQUEST_INTERNAL_ID       = 0x11
CMD_REQUEST_FIRMWARE_WRITE_DATE = 0x12
CMD_REQUEST_DEVICE_VENDOR     = 0x13
CMD_REQUEST_DEVICE_NAME       = 0x14
CMD_REQUEST_CURRENT_STATE     = 0x15
CMD_REQUEST_OUTPUT_RANDOM_NUMBER = 0xFA # NEW
```

Then, add the following processing logic:

```python
if command == CMD_REQUEST_OUTPUT_RANDOM_NUMBER:
    print("\n[INFO] RANDOM NUMBER Request")
    message = hex_to_ascii(data.hex())
    print(message)
```

This completes the implementation on the primary side.

## secondary

Next, we explain how to add the command for the secondary setup.

### Modify `serial_secondary.py`

Define the command constant:

```python
# === COMMAND DEFINITIONS ===
CMD_PING                       = 0x00
CMD_INTERNAL_LED_ON_OFF       = 0x01
CMD_REBOOT_DEVICE             = 0x02
CMD_REQUEST_GENERAL_STATUS    = 0x03
CMD_REQUEST_FIRMWARE_VERSION  = 0x03
CMD_REQUEST_DEVICE_TICK       = 0x10
CMD_REQUEST_INTERNAL_ID       = 0x11
CMD_REQUEST_FIRMWARE_WRITE_DATE = 0x12
CMD_REQUEST_DEVICE_VENDOR     = 0x13
CMD_REQUEST_DEVICE_NAME       = 0x14
CMD_REQUEST_CURRENT_STATE     = 0x15
CMD_REQUEST_IMU_SENSOR_DATA   = 0x20      
CMD_REQUEST_OUTPUT_RANDOM_NUMBER = 0xFA # NEW  
```

Then add the same processing logic as in the primary:

```python
# REQUEST OUTPUT RANDOM NUMBER
if command == CMD_REQUEST_OUTPUT_RANDOM_NUMBER:
    print("\n[INFO] RANDOM NUMBER Request")
    message = hex_to_ascii(data.hex())
    print(message)
```

### Add M5Stack-Side Processing

First, define the command constant in `protocol_definitions.h`, same as the primary:

```cpp
#define CMD_REQUEST_OUTPUT_RANDOM_NUMBER    0xFA // NEW
```

Then add the same logic as primary in `command_handler.hpp`:

```cpp
case CMD_REQUEST_OUTPUT_RANDOM_NUMBER: {
    uint32_t rand_number = random(10000000, 100000000);

    char message[13];
    snprintf(message, sizeof(message), "rnd_%08lu", (unsigned long)rand_number);

    uint8_t send_data[16];
    uint8_t send_data_len = 0;
    convert_date_to_ascii_array(message, send_data, &send_data_len, sizeof(send_data));

    pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, ERR_SUCCESS, mode);
    break;
  }
```

This completes the implementation for the secondary system.
