# PREEMPT_RT対応 リアルタイムGPIO制御プログラム

このプロジェクトは、PREEMPT_RTカーネルを使用した高精度なリアルタイムGPIO制御プログラムです。

## PREEMPT_RTとは

PREEMPT_RTは、LinuxカーネルをリアルタイムOS化するパッチセットです。これにより、ハードリアルタイム性能が要求されるアプリケーションでも、決定論的な応答時間を実現できます。

## リアルタイム機能

### 1. リアルタイムスケジューリング
- **SCHED_FIFO**: 先入れ先出しスケジューリングポリシー
- **優先度設定**: 1-99の範囲で設定（数値が大きいほど優先度が高い）
- LED制御: 優先度80
- ボタン入力: 優先度70

### 2. メモリロック
- `mlockall(MCL_CURRENT | MCL_FUTURE)`: プロセスのメモリをロックし、スワップを防止
- ページフォルトによる遅延を排除

### 3. 高精度タイマー
- `clock_nanosleep(CLOCK_MONOTONIC, ...)`: ナノ秒精度のスリープ
- `clock_gettime(CLOCK_MONOTONIC, ...)`: ナノ秒精度の時刻取得
- 従来の`usleep()`より高精度で決定論的

## プログラム一覧

### 1. led_control - リアルタイムLED制御
GPIO17のLEDを高精度タイミングで制御します。

**機能:**
- 1msの制御周期（変更可能）
- 累積誤差の測定と表示
- Ctrl+Cでの安全な終了

**実行:**
```bash
sudo ./led_control
```

**出力例:**
```
=== PREEMPT_RT対応 リアルタイムLED制御プログラム ===
制御周期: 1000000 ns (1.000 ms)
リアルタイムスレッド設定完了 (優先度: 80, SCHED_FIFO)
GPIO17の設定完了
LED制御を開始します。Ctrl+Cで終了します

サイクル: 10000, 累積誤差: 245 ns (0.245 us), 平均誤差: 0.025 ns
サイクル: 20000, 累積誤差: 512 ns (0.512 us), 平均誤差: 0.026 ns
```

### 2. button_input - リアルタイムボタン入力検出
GPIO27のボタン入力を高精度で検出します。

**機能:**
- 1msのポーリング間隔
- ナノ秒精度のタイムスタンプ
- ボタン押下時間の測定

**実行:**
```bash
sudo ./button_input
```

**出力例:**
```
=== PREEMPT_RT対応 リアルタイムボタン入力検出プログラム ===
ポーリング間隔: 1.000 ms
リアルタイムスレッド設定完了 (優先度: 70, SCHED_FIFO)
GPIO27の設定完了
ボタン入力を監視しています。Ctrl+Cで終了します

[1] ボタンが押されました (時刻: 1700000000.123456789)
[1] ボタンが離されました (押下時間: 245.123 ms)
```

## ビルド

```bash
make all
```

## 実行要件

### 必須
- Raspberry Pi（任意のモデル）
- PREEMPT_RTカーネル（推奨）
- sudo権限（リアルタイムスケジューリングとメモリロックに必要）

### PREEMPT_RTカーネルの確認

```bash
uname -a | grep PREEMPT
```

出力に`PREEMPT RT`が含まれていればPREEMPT_RTカーネルが動作しています。

### PREEMPT_RTカーネルのインストール（Raspberry Pi OS）

```bash
# カーネルの更新
sudo rpi-update

# または、公式のRTカーネルをインストール
sudo apt-get install linux-image-rt-arm64
```

## カスタマイズ

### 制御周期の変更

`led_control.c`の`INTERVAL_NS`を変更：
```c
#define INTERVAL_NS 1000000  // 1ms
#define INTERVAL_NS 500000   // 0.5ms
#define INTERVAL_NS 100000   // 0.1ms (100us)
```

### リアルタイム優先度の変更

```c
#define RT_PRIORITY 80  // 1-99の範囲で設定
```

**注意:** 優先度99は最高優先度で、システムの重要なプロセスと競合する可能性があります。通常は70-90の範囲を推奨します。

## パフォーマンス測定

### レイテンシの測定

```bash
# cyclictestをインストール
sudo apt-get install rt-tests

# レイテンシテストを実行（1時間）
sudo cyclictest -t1 -p80 -n -i1000 -l3600000
```

### CPUアフィニティの設定

特定のCPUコアに固定することで、さらに決定論的な動作を実現できます：

```c
#include <sched.h>

cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(3, &cpuset);  // CPU3に固定
sched_setaffinity(0, sizeof(cpuset), &cpuset);
```

## トラブルシューティング

### "Permission denied" エラー
```bash
sudo ./led_control
```
sudo権限で実行してください。

### "メモリロックに失敗" エラー
リソース制限を確認：
```bash
ulimit -l
# unlimited でない場合
ulimit -l unlimited
```

### GPIO制御ができない
GPIOデバイスのパーミッションを確認：
```bash
ls -l /dev/gpiochip0
sudo chmod 666 /dev/gpiochip0
```

### 高い優先度でカーネルパニック
優先度を下げて再試行：
```c
#define RT_PRIORITY 50  // 低めに設定
```

## 回路図

### LED制御（GPIO17）
```
GPIO17 --- 330Ω抵抗 --- LED(赤) --- GND
```

### ボタン入力（GPIO27）
```
GPIO27 --- タクトスイッチ --- GND
(内部プルアップ有効)
```

## 参考資料

- [PREEMPT_RT Wiki](https://wiki.linuxfoundation.org/realtime/start)
- [Raspberry Pi GPIO](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio)
- [Real-Time Linux](https://rt.wiki.kernel.org/)

## ライセンス

MIT License
