import serial

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
        head = ser.read(1)
        if head != b'#':
            continue

        # 固定ヘッダーを受信後、残りを受信
        header = head[0]
        ids = ser.read(3)
        if len(ids) < 3:
            print("ヘッダー直後のフィールド不足")
            continue

        device_id, target_id, command = ids
        data = bytearray()

        # 可変長受信（最大245）
        while True:
            b = ser.read(1)
            if not b:
                break
            if b[0] == 0x0D:  # フッターなら終了
                break
            data.append(b[0])

        if len(data) < 1:
            print("データ部 or CRC不足")
            continue

        # CRC = データ末尾
        crc_received = data[-1]
        payload = data[:-1]
        crc_input = bytes([device_id, target_id, command]) + payload
        crc_calc = crc8_dallas_maxim(crc_input)

        if crc_calc == crc_received:
            print("✅ 受信成功：DevID={}, TgtID={}, Cmd=0x{:02X}, Data={}".format(device_id, target_id, command, list(payload)))
        else:
            print("❌ CRCエラー")

if __name__ == '__main__':
    main()
