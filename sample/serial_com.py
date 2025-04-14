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
    ser = serial.Serial('COM12', 115200, timeout=1)
    print("Listening on COM12...")

    while True:
        # ─────────────────────────────
        # 送信処理：コマンドをM5Stackへ送る
        # ─────────────────────────────
        device_id = 0x01
        target_id = 0x02
        command = 0x03
        payload = b''  # 今回はなし
        crc_input = bytes([device_id, target_id, command]) + payload
        crc = crc8_dallas_maxim(crc_input)
        packet = b'#' + bytes([device_id, target_id, command, crc]) + b'\r'
        ser.write(packet)
        print(f"📤 Sent command: {packet.hex()}")

        # ─────────────────────────────
        # 受信処理：M5Stackからの応答を待つ
        # 応答形式: OwnID + Status + CRC + \r
        # ─────────────────────────────
        response = ser.read(4)
        if len(response) == 4:
            own_id, status, crc_recv, footer = response
            if footer != 0x0D:
                print("❌ 応答フッターが不正")
                continue

            crc_calc = crc8_dallas_maxim(bytes([own_id, status]))
            if crc_calc == crc_recv:
                print(f"✅ 応答受信: OwnID={own_id}, Status=0x{status:02X}")
            else:
                print("❌ CRC不一致（応答）")
        else:
            print("❌ 応答データ不足")

        time.sleep(1)

if __name__ == '__main__':
    main()
