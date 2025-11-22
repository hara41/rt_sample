# Raspberry Pi C言語プログラム

このプロジェクトはRaspberry Piで動作するC言語プログラムのサンプル集です。

## 必要な環境

- Raspberry Pi（任意のモデル）
- Raspbian/Raspberry Pi OS
- GCC コンパイラ
- GPIO制御用ライブラリ（WiringPi または pigpio）

## セットアップ

### 1. 必要なパッケージのインストール

```bash
sudo apt-get update
sudo apt-get install -y build-essential git
```

### 2. GPIO制御ライブラリのインストール

**WiringPiを使用する場合：**
```bash
sudo apt-get install wiringpi
```

**pigpioを使用する場合：**
```bash
sudo apt-get install pigpio python3-pigpio
sudo systemctl enable pigpiod
sudo systemctl start pigpiod
```

## プログラム一覧

### 1. LED制御プログラム (`led_control.c`)
GPIO17のLEDをON/OFF制御します。

**ビルド:**
```bash
make led_control
```

**実行:**
```bash
sudo ./led_control
```

### 2. 温度センサー読取り (`temperature_sensor.c`)
I2C接続の温度センサーから温度を読み取ります。

**ビルド:**
```bash
make temperature_sensor
```

**実行:**
```bash
sudo ./temperature_sensor
```

### 3. ボタン入力検出 (`button_input.c`)
GPIO27のボタン入力を検出します。

**ビルド:**
```bash
make button_input
```

**実行:**
```bash
sudo ./button_input
```

## すべてのプログラムをビルド

```bash
make all
```

## クリーンアップ

```bash
make clean
```

## 回路図

### LED制御
```
GPIO17 --- 330Ω抵抗 --- LED --- GND
```

### ボタン入力
```
GPIO27 --- ボタン --- GND
(GPIO27に内部プルアップを設定)
```

## トラブルシューティング

- **Permission denied**: `sudo`コマンドで実行してください
- **ライブラリが見つからない**: `apt-get install`でライブラリを再インストールしてください
- **I2C接続エラー**: `i2cdetect -y 1`でデバイスが認識されているか確認してください

## ライセンス

MIT License
