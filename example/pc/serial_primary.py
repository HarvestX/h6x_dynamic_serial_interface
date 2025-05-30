import serial
import time
import threading
import struct
import sys
import subprocess
import cpuinfo


# === DEVICE ID ===
DEVICE_ID = 0x01

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
CMD_REQUEST_CPU_USAGE         = 0x20
CMD_REQUEST_OUTPUT_RANDOM_NUMBER = 0xFA #NEW

# === COLOR DEFINITIONS ===
RED_COMMAND = 1
GREEN_COMMAND = 0
RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"

# === Global Variables ===
global command
global print_color
data_bytes = b''
print_color = 0


def crc8_dallas_maxim(data: bytes) -> int:
    crc = 0x00
    for byte in data:
        extract = byte
        for _ in range(8):
            sum_ = (crc ^ extract) & 0x01
            crc >>= 1
            if sum_:
                crc ^= 0x8C
            extract >>= 1
    return crc

def hex_to_ascii(hex_str):
    try:
        chars = [chr(int(hex_str[i:i+2], 16)) for i in range(0, len(hex_str), 2)]
        return ''.join(chars)
    except ValueError as e:
        print(f"ascii error: {e}")
        return None

def send_packet(ser):
    target_id = 0x01
    global data_bytes
    global command
    payload = b''
    crc_input = bytes([target_id, command, len(data_bytes)]) + data_bytes + payload
    crc = crc8_dallas_maxim(crc_input)
    packet = b'#' + bytes([target_id, command, len(data_bytes)]) + data_bytes + bytes([crc]) + b'\r'
    ser.write(packet)

    print("\n=== Sent Packet ===")
    print(f"Command     : 0x{command:02X}")
    print(f"Packet (hex): {packet.hex()}")

def receive_response_thread(ser):
    while True:
        global data_bytes, command
        head = ser.read(1)
        if head != b'$':
            continue

        response = ser.read(4)
        if len(response) != 4:
            continue

        own_id, command, status, length = response
        data = ser.read(length)
        data_bytes = data
        response_2 = ser.read(2)
        if len(response_2) != 2:
            continue
        crc_recv, footer = response_2

        global print_color
        if print_color == RED_COMMAND:
            print(f"\n{RED}=== Recieved data from M5Stack ==={RESET}")
        elif print_color == GREEN_COMMAND:
            print(f"\n{GREEN}=== Recieved data from M5Stack ==={RESET}")
        else:
            print("\n=== Recieved data from M5Stack ===")

        

        print(f"Own ID      : 0x{own_id:02X}")
        print(f"Command     : 0x{command:02X}")
        print(f"Status      : 0x{status:02X}")
        print(f"Data Length : {length}")
        print(f"Data Bytes  : {' '.join(f'0x{b:02X}' for b in data)}")
        print(f"CRC (Recv)  : 0x{crc_recv:02X}")
        print(f"Footer      : 0x{footer:02X}")

        # PRINT_COLOR
        if command == CMD_INTERNAL_LED_ON_OFF:
            if print_color == GREEN_COMMAND:
                print(f"\n[INFO] {RED}Change LED Color: Red{RESET}")
                print_color = RED_COMMAND
            elif print_color == RED_COMMAND:
                print(f"\n[INFO] {GREEN}Change LED Color: Green{RESET}")
                print_color = GREEN_COMMAND
            else:
                print(f"\n[INFO] LED Color: Unknown")


        # REBOOT
        if command == CMD_REBOOT_DEVICE:
            print("\n[INFO] Device is rebooting...")
            subprocess.Popen([sys.executable] + sys.argv)
            sys.exit()

        # REQUEST GENERAL STATUS
        if command == CMD_REQUEST_GENERAL_STATUS:
            print("\n[INFO] General Status Request")
            if len(data) > 0:
                print(f"  Status: 0x{data[0]:02X}")
            else:
                print("  [ERROR] No status data received.")

        # REQUEST FIRMWARE VERSION
        if command == CMD_REQUEST_FIRMWARE_VERSION:
            print("\n[INFO] Firmware Version Request")
            if len(data) > 0:
                print(f"  Version: 0x{data[1]:02X}")
            else:
                print("  [ERROR] No version data received.")

        # REQUEST DEVICE TICK
        if command == CMD_REQUEST_DEVICE_TICK:
            print("\n[INFO] Device Tick Request")
            hex_str = data.hex()
            print(f"  Raw (hex) : {hex_str}")

            if len(data) > 0:
                tick = int(hex_str, 16)
                print(f"  Tick (dec): {tick}")
            else:
                print("  [ERROR] No data received.")

        # REQUEST FIRMWARE WRITE DATE
        if command == CMD_REQUEST_FIRMWARE_WRITE_DATE:
            print("\n[INFO] Firmware Write Date Request")
            date = hex_to_ascii(data.hex())
            print(f"  Date: {date}")
        
        # REQUEST DEVICE VENDOR
        if command == CMD_REQUEST_DEVICE_VENDOR:
            print("\n[INFO] Device Vendor Request")
            vendor = hex_to_ascii(data.hex())
            print(f"  Vendor: {vendor}")
            cpuinfo.get_cpu_info()['vendor_id_raw']

        # REQUEST DEVICE NAME
        if command == CMD_REQUEST_DEVICE_NAME:
            print("\n[INFO] Device Name Request")
            name = hex_to_ascii(data.hex())
            print(f"  Name: {name}")
        
        # REQUEST CURRENT STATE
        if command == CMD_REQUEST_CURRENT_STATE:
            print("\n[INFO] Current State Request")
            print(f"  State: {data.hex()}")
        
        # REQUEST CPU Usage
        if command == CMD_REQUEST_CPU_USAGE:
            print("\n[INFO] CPU Usage")
            if len(data) != 4:
                print("  [ERROR] Invalid data length for CPU usage.")
            else:
                usage_percent = struct.unpack('<f', data)[0]
                print(f"  Usage: {usage_percent:.2f} %")

        if command == CMD_REQUEST_OUTPUT_RANDOM_NUMBER:
            print("\n[INFO] RANDOM NUMBER Request")
            message = hex_to_ascii(data.hex())
            print(message)
            
        crc_calc = crc8_dallas_maxim(bytes([own_id, command, status, length]) + data)
        print("\n[CRC Check]")
        if crc_calc == crc_recv:
            print(f"  Success: CRC matched (0x{crc_recv:02X})")
        else:
            print(f"  Failure: CRC mismatch")
            print(f"    Calculated: 0x{crc_calc:02X}")
            print(f"    Received  : 0x{crc_recv:02X}")

        # Send a packet back to the device
        send_packet(ser)

def main():
    global command
    ser = serial.Serial('COM12', 115200, timeout=1)
    print("=== Listening on COM12 ===")

    threading.Thread(target=receive_response_thread, args=(ser,), daemon=True).start()

    while True:
        time.sleep(1)

if __name__ == '__main__':
    main()
