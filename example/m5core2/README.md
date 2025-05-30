# コマンドの追加方法
コマンドの追加方法について解説する。追加するコマンドの内容は以下の通りである。
```
コマンド番号：250
返却内容：rnd_  + 8個の数字列をランダムに生成したもののASCII化（例："rnd_48314168"）
```

## primary

### M5Stack出力用の配列に追加
example\m5core2\primary\main.cppの以下の部分に、M5Stackの選択画面で表示したい文字列を追加する。
```C++
const char* options[] = {
  "00: Ping", "01: Change print color (red/green)", "02: Reboot device",
  "03: Request general status", "03: Request firmware version",
  "10: Request device tick", "12: Request firmware write date",
  "13: Request device vendor", "14: Request device name",
  "15: Request current state", "20: Display CPU usage",
  "250: Request output random number" // NEW
};
```

同様に以下の配列にも新しいコマンド番号を追加する。
```C++
const uint8_t return_values[] = {
  0x00, 0x01, 0x02, 0x03, 0x03, 0x10, 0x12, 0x13, 0x14, 0x15, 0x20, 0x250 // NEW
};
```

