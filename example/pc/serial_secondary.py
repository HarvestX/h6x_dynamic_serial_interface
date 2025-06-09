import serial
import time
import threading
import struct
import sys
import subprocess
import cpuinfo


# === DEVICE ID ===
TARGET_ID = 0x01

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
CMD_REQUEST_CARRIPLATION_STATUS  = 0x21   
CMD_REQUEST_CARRIPLATION_EXECUSION = 0x96   
CMD_REQUEST_OUTPUT_RANDOM_NUMBER = 0xFA #NEW           

command = CMD_REQUEST_DEVICE_TICK

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

def command_input_thread():
    global command
    while True:
        user_input = input("Enter command (e.g., 00, 01, 10): ").strip()
        try:
            new_command = int(user_input, 16)
            command = new_command
            print(f"[INFO] Command updated to: 0x{command:02X}")
        except ValueError:
            print("[ERROR] Invalid input. Please enter a valid hex value like 00, 01, 10.")

def send_packet_thread(ser):
    global command
    while True:
        header = ord('#')
        target_id = TARGET_ID
        data_bytes = bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08])
        payload = b''
        crc_input = bytes([header, target_id, command, len(data_bytes)]) + data_bytes + payload
        crc = crc8_dallas_maxim(crc_input)
        packet = bytes([header, target_id, command, len(data_bytes)]) + data_bytes + bytes([crc]) + b'\r'
        ser.write(packet)

        print("\n=== Sent Packet ===")
        print(f"Command     : 0x{command:02X}")
        print(f"Packet (hex): {packet.hex()}")

        # time.sleep(0.1)
        time.sleep(1)

def receive_response_thread(ser):
    while True:
        head = ser.read(1)
        if head != b'$':
            continue

        response = ser.read(3)
        if len(response) != 3:
            continue

        own_id, status, length = response
        data = ser.read(length)
        response_2 = ser.read(2)
        if len(response_2) != 2:
            continue

        crc_recv, footer = response_2

        print("\n=== Received Packet ===")
        print(f"Own ID      : 0x{own_id:02X}")
        print(f"Status      : 0x{status:02X}")
        print(f"Data Length : {length}")
        print(f"Data Bytes  : {' '.join(f'0x{b:02X}' for b in data)}")
        print(f"CRC (Recv)  : 0x{crc_recv:02X}")
        print(f"Footer      : 0x{footer:02X}")

        # PING
        if command == CMD_PING:
            print("\n[INFO] Ping Response")
            print(f"  PING: {data}")

        # REBOOT
        if command == CMD_REBOOT_DEVICE:
            print("\n[INFO] Device is rebooting...")
            subprocess.Popen([sys.executable] + sys.argv)
            sys.exit()

        # REQUEST GENERAL STATUS
        if command == CMD_REQUEST_GENERAL_STATUS:
            print("\n[INFO] General Status Request")
            print(f"  Status: {status}")

        # REQUEST FIRMWARE VERSION
        if command == CMD_REQUEST_FIRMWARE_VERSION:
            print("\n[INFO] Firmware Version Request")
            version = data.decode('utf-8', errors='ignore')
            print(f"  Version: {version}")

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
            print(f"  State: {data}")

        # REQUEST IMU SENSOR DATA
        if command == CMD_REQUEST_IMU_SENSOR_DATA and len(data) == 16:
            roll, pitch, yaw, temp = struct.unpack('<ffff', data)
            print("\n[Sensor Values]")
            print(f"  Roll : {roll:.2f}")
            print(f"  Pitch: {pitch:.2f}")
            print(f"  Yaw  : {yaw:.2f}")
            print(f"  Temp : {temp:.2f}")

        # REQUEST CARRIPLATION STATUS
        if command == CMD_REQUEST_CARRIPLATION_STATUS:
            print("\n[INFO] Carriplation Status Request")
            if len(data) > 0:
                if status == 0x00:
                    print("  Status: Not yet started")
                elif status == 0x01:
                    print("  Status: In progress")
                elif status == 0x02:
                    print("  Status: Completed")
                else:
                    print(f"  Status: Unknown (0x{status:02X})")

        # REQUEST CARRIPLATION EXECUSION
        if command == CMD_REQUEST_CARRIPLATION_EXECUSION:
            print("\n[INFO] Carriplation Execution Request")

        # REQUEST OUTPUT RANDOM NUMBER
        if command == CMD_REQUEST_OUTPUT_RANDOM_NUMBER:
            print("\n[INFO] RANDOM NUMBER Request")
            message = hex_to_ascii(data.hex())
            print(message)

        # CRC Check
        crc_calc = crc8_dallas_maxim(bytes([ord('$'), own_id, status, length]) + data)
        print("\n[CRC Check]")
        if crc_calc == crc_recv:
            print(f"  Success: CRC matched (0x{crc_recv:02X})")
        else:
            print(f"  Failure: CRC mismatch")
            print(f"    Calculated: 0x{crc_calc:02X}")
            print(f"    Received  : 0x{crc_recv:02X}")

def main():
    global command
    ser = serial.Serial('COM12', 115200, timeout=1)
    print("=== Listening on COM12 ===")

    threading.Thread(target=command_input_thread, daemon=True).start()
    threading.Thread(target=send_packet_thread, args=(ser,), daemon=True).start()
    threading.Thread(target=receive_response_thread, args=(ser,), daemon=True).start()

    while True:
        time.sleep(1)

if __name__ == '__main__':
    main()
