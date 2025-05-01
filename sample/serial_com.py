import serial
import time
import threading
import struct

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

command = CMD_PING  # 初期コマンド

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
        print(f"変換エラー: {e}")
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

def main():
    global command
    ser = serial.Serial('COM12', 115200, timeout=1)
    print("=== Listening on COM12 ===")

    # スレッド開始
    threading.Thread(target=command_input_thread, daemon=True).start()

    while True:
        target_id = 0x01
        data_bytes = bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08])
        payload = b''
        crc_input = bytes([target_id, command, len(data_bytes)]) + data_bytes + payload
        crc = crc8_dallas_maxim(crc_input)
        packet = b'#' + bytes([target_id, command, len(data_bytes)]) + data_bytes + bytes([crc]) + b'\r'
        ser.write(packet)

        print("\n=== Sent Packet ===")
        print(f"Target ID   : 0x{target_id:02X}")
        print(f"Command     : 0x{command:02X}")
        print(f"Data Length : {len(data_bytes)}")
        print(f"Data Bytes  : {' '.join(f'0x{b:02X}' for b in data_bytes)}")
        print(f"CRC         : 0x{crc:02X}")
        print(f"Packet (hex): {packet.hex()}")

        head = ser.read(1)
        if head != b'$':
            print("=== Response Error ===")
            print("Invalid response header")
            time.sleep(0.5)
            continue

        response = ser.read(3)
        if len(response) == 3:
            own_id, status, length = response
            data = ser.read(length)
            response_2 = ser.read(2)
            if len(response_2) != 2:
                print("=== Response Error ===")
                print("Footer or CRC missing")
                continue

            crc_recv, footer = response_2
            print("\n=== Received Packet ===")
            print(f"Own ID      : 0x{own_id:02X}")
            print(f"Status      : 0x{status:02X}")
            print(f"Data Length : {length}")
            print(f"Data Bytes  : {' '.join(f'0x{b:02X}' for b in data)}")
            print(f"CRC (Recv)  : 0x{crc_recv:02X}")
            print(f"Footer      : 0x{footer:02X}")

            if command == 0x20 and len(data) == 16:
                roll, pitch, yaw, temp = struct.unpack('<ffff', data)
                print("\n[Sensor Values]")
                print(f"  Roll : {roll:.2f}")
                print(f"  Pitch: {pitch:.2f}")
                print(f"  Yaw  : {yaw:.2f}")
                print(f"  Temp : {temp:.2f}")

            if command in [0x12, 0x13, 0x14]:
                ascii_output = hex_to_ascii(data.hex())
                print(f"\n[ASCII Output] {ascii_output}")

            if footer != 0x0D:
                print("=== Response Error ===")
                print("Incorrect response footer")
                continue

            crc_calc = crc8_dallas_maxim(bytes([own_id, status, length]) + data)
            print("\n[CRC Check]")
            if crc_calc == crc_recv:
                print(f"  Success: CRC matched (0x{crc_recv:02X})")
            else:
                print(f"  Failure: CRC mismatch")
                print(f"    Calculated: 0x{crc_calc:02X}")
                print(f"    Received  : 0x{crc_recv:02X}")
        else:
            print("=== Response Error ===")
            print("Response too short")

        time.sleep(1)


if __name__ == '__main__':
    main()
