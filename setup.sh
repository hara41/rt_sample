#!/bin/bash

# Raspberry Pi C言語開発環境セットアップスクリプト

echo "=========================================="
echo "Raspberry Pi C言語開発環境セットアップ"
echo "=========================================="
echo ""

# ユーザー確認
if [ "$EUID" -ne 0 ]; then 
    echo "このスクリプトはsudoで実行してください"
    exit 1
fi

echo "1. システムパッケージのアップデート中..."
apt-get update
apt-get upgrade -y

echo ""
echo "2. ビルドツールのインストール中..."
apt-get install -y build-essential git gcc make

echo ""
echo "3. GPIO制御ライブラリのインストール中..."
apt-get install -y wiringpi

echo ""
echo "4. I2C通信ライブラリのインストール中..."
apt-get install -y i2c-tools libi2c-dev

echo ""
echo "=========================================="
echo "セットアップが完了しました！"
echo "=========================================="
echo ""
echo "次のコマンドでプログラムをビルドできます："
echo "  cd /home/hara41/rt_sample"
echo "  make all"
echo ""
echo "実行例："
echo "  sudo ./led_control"
echo "  sudo ./button_input"
echo "  sudo ./temperature_sensor"
echo ""
