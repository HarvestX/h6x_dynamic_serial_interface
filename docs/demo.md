# H6X Dynamic Serial Interface Demo

## Overview

This demo demonstrates the H6X Dynamic Serial Interface protocol using the packet calculation GUI tool. The demo creates a virtual serial interface connection between two instances of the packet_calc_gui application - one acting as a host and another as a client.

## Prerequisites

### System Requirements
- ROS 2 Humble or later
- socat (for virtual serial port creation)

### Installation

ROS 2 package dependencies must be installed before running the demo. The following commands will install the necessary dependencies and build the application:

```bash
source /opt/ros/humble/setup.bash

mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone https://github.com/HarvestX/h6x_dynamic_serial_interface.git

cd ~/ros2_ws/
rosdep install -y --from-paths . --ignore-src

colcon build --symlink-install
source install/setup.bash
```

Install `socat` if not already installed:

```bash
sudo apt install -y socat
```

## Demo Setup

### Step 1: Create Virtual Serial Port Pair

Create a virtual serial port pair using socat:

```bash
# Create virtual serial port pair
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

This command will output something like:
```
2025/01/15 10:30:45 socat[12345] N PTY is /dev/pts/2
2025/01/15 10:30:45 socat[12346] N PTY is /dev/pts/3
```

Note the two PTY devices (e.g., `/dev/pts/2` and `/dev/pts/3`) - these will be used for the host and client connections.

### Step 2: Launch Host Application

In a new terminal, launch the host instance:

```bash
cd ~/ros2_ws/
source install/setup.bash

ros2 run h6x_dynamic_packet_tools packet_calc_gui --ros-args -p role:="host"
```

**Configuration:**
1. Select the first virtual port (e.g., `/dev/pts/2`) from the Port dropdown
2. Set baud rate to 9600 (default)
3. Click "Connect" to establish connection
4. The connection status should show "Connected" in green

### Step 3: Launch Client Application

In another terminal, launch the client instance:

```bash
cd h6x_dynamic_packet_tools/build
./packet_calc_gui --ros-args -p role:="client"
```

**Configuration:**
1. Select the second virtual port (e.g., `/dev/pts/3`) from the Port dropdown
2. Set baud rate to 9600 (default)
3. Click "Connect" to establish connection
4. The connection status should show "Connected" in green

## Demo Test Scenarios

### Test 1: Basic Ping Command

**Host Side:**
1. Set Client ID: `1`
2. Set Command: `0` (CMD_PING)
3. Enter data: `0` (single byte ping data)
4. Click "Calculate Packet" to verify CRC calculation
5. Click "Send Custom Packet" to transmit

**Expected Client Response:**
- Status: `0x00` (ERR_SUCCESS)
- Data: Echo of ping data
- CRC validation successful

**Log Output Example:**
```
[10:31:20] Connected to device on /dev/pts/2
[10:31:25] Custom packet sent successfully - Command: 0x00, Response length: 1
[10:31:25] Data received - Command: 0x00, Status: 0x00, Data length: 1 Data: 0 ASCII: [0x00]
```

### Test 2: LED Control Command

**Host Side:**
1. Set Client ID: `1`
2. Set Command: `1` (CMD_INTERNAL_LED_ON_OFF)
3. Enter data: `1` (LED on) or `0` (LED off)
4. Send packet

**Expected Client Response:**
- Status: `0x00` (ERR_SUCCESS)
- Data: Confirmation of LED state change

### Test 3: Device Information Request

**Host Side:**
1. Set Client ID: `1`
2. Set Command: `13` (CMD_REQUEST_DEVICE_VENDOR)
3. Enter data: `0` (no additional data required)
4. Send packet

**Expected Client Response:**
- Status: `0x00` (ERR_SUCCESS)
- Data: ASCII string containing device vendor information
- ASCII display shows readable vendor name

### Test 4: Custom Data Transmission

**Host Side:**
1. Set Client ID: `1`
2. Set Command: `250` (CMD_REQUEST_OUTPUT_RANDOM_NUMBER)
3. Enter data: `72,101,108,108,111` (ASCII for "Hello")
4. Send packet

**Expected Client Response:**
- Status: `0x00` (ERR_SUCCESS)
- Data: Processed response based on custom command
- ASCII display shows readable text if applicable

## GUI Interface Guide

### Connection Panel
- **Port**: Select virtual serial port
- **Baud Rate**: Communication speed (default: 9600)
- **Connect/Disconnect**: Establish/terminate connection
- **Status Indicator**: Green (Connected) / Red (Disconnected)

### Packet Calculator Panel
- **Client ID**: Target device identifier (1-255)
- **Command**: Command code to send (0-255)
- **Packet Data**: Comma-separated data bytes
- **Calculated Length**: Automatically computed data length
- **Calculated CRC**: CRC-8 checksum for error detection

### Response Panel
- **Mode**: Response packet mode indicator
- **CRC**: Received CRC value
- **Data**: Raw numeric data received
- **Data (ASCII)**: ASCII interpretation of received data

### Log Panel
- Real-time communication log with timestamps
- Packet transmission confirmations
- Error messages and status updates

## Packet Format Verification

The demo allows verification of the protocol specification:

### Sent Packet Structure
```
Host Packet: # [Client ID] [Command] [Data Length] [Data...] [CRC8]
Example:     # 01 00 01 00 8A
```

### Received Packet Structure
```
Client Packet: $ [Client ID] [Status] [Data Length] [Data...] [CRC8]
Example:       $ 01 00 01 00 15
```

## Troubleshooting

### Common Issues

1. **"No serial ports found"**
   - Ensure virtual serial ports are created with socat
   - Check port permissions: `sudo chmod 666 /dev/pts/X`

2. **Connection timeout**
   - Verify both applications are using paired ports
   - Check that socat process is still running
   - Restart virtual port pair if necessary

3. **CRC mismatch errors**
   - Verify data format (comma-separated integers)
   - Check that both sides use same client ID
   - Ensure no data corruption in virtual link

4. **No response from client**
   - Confirm client application is properly connected
   - Check that client implements the requested command
   - Verify packet format compliance

### Debug Commands

Check virtual ports:
```bash
ls -la /dev/pts/
```

Monitor serial traffic:
```bash
# Terminal 1 - Monitor host side
cat /dev/pts/2

# Terminal 2 - Monitor client side  
cat /dev/pts/3
```

View socat process:
```bash
ps aux | grep socat
```

## Protocol Validation

This demo validates:
- Variable-length packet transmission
- CRC-8 error detection
- Host-client communication pattern
- Command/status code handling
- ASCII data interpretation
- Real-time bidirectional communication

## Extending the Demo

### Adding Custom Commands

1. Define new command codes in protocol_definitions.h
2. Implement command handlers in client application
3. Test with host GUI using custom command codes

### Performance Testing

1. Send multiple rapid packets to test throughput
2. Vary data payload sizes (1-251 bytes)
3. Monitor timing and response latency

### Error Condition Testing

1. Send invalid CRC packets
2. Test with corrupted data
3. Simulate timeout conditions
4. Verify error handling responses

## Notes

- Current implementation is host-centric (GUI primarily for host operations)
- Timeout handling is not fully implemented but can be ignored for demo purposes
- The demo assumes both instances run on the same Linux machine
- For production use, consider implementing proper client-side GUI and timeout mechanisms