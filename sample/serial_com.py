# protocol_definitions.py

# === DEVICE ID ===
DEVICE_ID = 0x01

# === COMMAND DEFINITIONS ===
CMD_PING                       = 0x00
CMD_INTERNAL_LED_ON_OFF       = 0x01
CMD_REBOOT_DEVICE             = 0x02
CMD_REQUEST_GENERAL_STATUS    = 0x03  # これは CMD_REQUEST_FIRMWARE_VERSION と同じ値
CMD_REQUEST_FIRMWARE_VERSION  = 0x03
CMD_REQUEST_DEVICE_TICK       = 0x10
CMD_REQUEST_INTERNAL_ID       = 0x11
CMD_REQUEST_FIRMWARE_WRITE_DATE = 0x12
CMD_REQUEST_DEVICE_VENDOR     = 0x13
CMD_REQUEST_DEVICE_NAME       = 0x14
CMD_REQUEST_CURRENT_STATE     = 0x15

# === ERROR CODES ===
ERR_SUCCESS         = 0x00
ERR_FAILURE         = 0x01
ERR_UNKNOWN_COMMAND = 0x02
ERR_CRC_ERROR       = 0x03
ERR_TIMEOUT         = 0x04
ERR_BUSY            = 0x05
ERR_BUFFER_FULL     = 0x06
ERR_OTHER           = 0xFF


import serial
import time

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

def main():
    MODE = 0
    ser = serial.Serial('COM12', 115200, timeout=1)
    print("Listening on COM12...")

    while True:
        # ─────────────────────────────
        # 送信処理：コマンドをM5Stackへ送る
        # ─────────────────────────────
        target_id = 0x01
        command = CMD_PING
        data_bytes = bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08])
        payload = b''
        crc_input = bytes([target_id, command, len(data_bytes)]) + data_bytes + payload
        crc = crc8_dallas_maxim(crc_input)
        packet = b'#' + bytes([target_id, command, len(data_bytes)]) + data_bytes + bytes([crc]) + b'\r'
        ser.write(packet)
        print(f"Sent command: {packet.hex()}")

        # ─────────────────────────────
        # 受信処理：M5Stackからの応答を待つ
        # 応答形式: OwnID + Status + CRC + \r
        # ─────────────────────────────
        head = ser.read(1)
        print(f"Received header: {head.hex()}")
        if head != b'$':
            print("Invalid response header")
            continue
        own_id = head

        response = ser.read(3)
        if len(response) == 3:
            own_id, status, length = response
            data = ser.read(length)
            print(f"Received: {response.hex()}{data.hex()}")
            response_2 = ser.read(2)
            crc_recv, footer = response_2
            print(f"Received: {response.hex()}{data.hex()}{response_2.hex()}")

            if footer != 0x0D:
                print("Incorrect response footer")
                continue

            crc_calc = crc8_dallas_maxim(bytes([own_id, status, length]) + data)
            if crc_calc == crc_recv:
                print(f"Acknowledgment: OwnID={own_id}, Status=0x{status:02X}, Length=0x{length:02X}, Data={data.hex()}, CRC=0x{crc_recv:02X}")
            else:
                print(f"CRC mismatch: calculated=0x{crc_calc:02X}, received=0x{crc_recv:02X}")

        else:
            print("Response data insufficiency")

        time.sleep(0.001)

if __name__ == '__main__':
    main()
