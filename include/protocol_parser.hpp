#ifndef PROTOCOL_PARSER_HPP
#define PROTOCOL_PARSER_HPP

#include <Arduino.h>
#include "crc8.hpp"  // crc8_calculate() を含むものが必要
#include "protocol_definitions.hpp"  // CMD_ / ERR_ の定義がここにある想定

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------
// プロトコルモードの種類
// ------------------------------
enum ProtocolMode {
    MODE_BASIC,         // 基本コマンド
    MODE_RECEIVE_DATA,  // データ要求コマンド
    MODE_SEND_DATA      // データ送信モード（PC → M5）
};

// ------------------------------
// 解析されたパケット情報構造体
// ------------------------------
struct ReceivedPacket {
    ProtocolMode mode;
    uint8_t src_device;   // device ID（送信元）
    uint8_t target_id;    // ターゲットID
    uint8_t command;      // コマンド
    const uint8_t* data;  // データ部（SEND時）
    uint8_t data_len;     // データ長
    bool crc_valid;       // CRC検証結果
};

// ------------------------------
// パケット解析処理
// ------------------------------
static inline ReceivedPacket parse_serial_packet(const uint8_t* buf, uint8_t len) {
    ReceivedPacket pkt = {};
    pkt.crc_valid = false;

    if (len < 4) {
        return pkt;  // 最低限の長さにも満たない
    }

    if (buf[0] != '#') {
        return pkt;  // ヘッダーが不正
    }

    // ───────────────────────────────
    // 形式1：基本/受信モード (# + ID + ID + CMD + CRC + \r)
    // ───────────────────────────────
    if (len == 6) {
        uint8_t device_id = buf[1];
        uint8_t target_id = buf[2];
        uint8_t command   = buf[3];
        uint8_t crc_recv  = buf[4];
        uint8_t footer    = buf[5];

        if (footer != '\r') return pkt;

        uint8_t calc_crc = crc8_calculate(&buf[1], 3);  // ID〜CMDの3バイト

        pkt.crc_valid = (crc_recv == calc_crc);
        pkt.src_device = device_id;
        pkt.target_id = target_id;
        pkt.command = command;

        if (command >= 0x20 && command <= 0x94) {
            pkt.mode = MODE_RECEIVE_DATA;
        } else {
            pkt.mode = MODE_BASIC;
        }

        return pkt;
    }

    // ───────────────────────────────
    // 形式2：送信モード（# + Data(n) + CRC + \r）
    // ───────────────────────────────
    if (len >= 4) {
        uint8_t crc_recv = buf[len - 2];
        uint8_t footer   = buf[len - 1];

        if (footer != '\r') return pkt;

        uint8_t calc_crc = crc8_calculate(&buf[1], len - 3);  // data部だけCRC

        pkt.crc_valid = (crc_recv == calc_crc);
        pkt.mode = MODE_SEND_DATA;
        pkt.data = &buf[1];
        pkt.data_len = len - 3;

        return pkt;
    }

    return pkt;  // ここには普通来ない
}


#ifdef __cplusplus
}
#endif

#endif  // PROTOCOL_PARSER_HPP
