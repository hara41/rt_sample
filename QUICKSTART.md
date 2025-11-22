# クイックスタートガイド

## 🚀 すぐに試す

### 1. 標準モードで実行（権限不要）
```bash
./led_control
```

**出力:**
```
⚠ リアルタイムスケジューリング失敗
  (sudo権限で実行するとリアルタイム性能が向上します)
部分的なリアルタイムモードで動作
```

メモリロックは有効ですが、リアルタイムスケジューリングは無効です。

---

### 2. フルリアルタイムモードで実行（sudo使用）
```bash
sudo ./led_control
```

**出力:**
```
✓ メモリロック有効
✓ リアルタイムスケジューリング有効 (優先度: 80, SCHED_FIFO)
✓ フルリアルタイムモードで動作
```

最高精度のリアルタイム制御が可能です。

---

### 3. sudo不要で実行できるようにする（推奨）

一般ユーザーにリアルタイム権限を付与：

```bash
sudo ./setup_rt_permission.sh
```

**ログアウト→ログイン後：**
```bash
./led_control  # sudoなしで実行可能！
```

---

## 📊 実行モード比較

| モード | コマンド | メモリロック | RTスケジューリング | 精度 |
|--------|----------|-------------|-------------------|------|
| 標準 | `./led_control` | ✓ | ✗ | 中 |
| フル | `sudo ./led_control` | ✓ | ✓ | 最高 |
| 権限設定後 | `./led_control` | ✓ | ✓ | 最高 |

---

## 🔧 PREEMPT_RTカーネルの確認

```bash
./check_rt.sh
```

✓ PREEMPT_RTカーネルが動作していることを確認済み

---

## 📝 プログラム一覧

### LED制御
```bash
sudo ./led_control
```
- 制御周期: 1ms (1,000,000 ns)
- GPIO17を使用
- 累積誤差を表示

### ボタン入力
```bash
sudo ./button_input
```
- ポーリング間隔: 1ms
- GPIO27を使用
- ナノ秒精度のタイムスタンプ

---

## ⚙️ カスタマイズ

### 制御周期を変更

`led_control.c` の 16行目を編集：
```c
#define INTERVAL_NS 1000000  // 1ms
```

変更例：
```c
#define INTERVAL_NS 500000   // 0.5ms (500us)
#define INTERVAL_NS 100000   // 0.1ms (100us)
#define INTERVAL_NS 10000    // 0.01ms (10us)
```

再ビルド：
```bash
make led_control
```

---

## 🎯 期待される性能

### 標準カーネル + usleep()
- 誤差: 数百μs 〜 数ms
- ジッター: 大きい

### PREEMPT_RT + clock_nanosleep() + sudo
- 誤差: 数百ns 〜 数μs
- ジッター: 非常に小さい
- **約1000倍の精度向上！**

---

## 💡 トラブルシューティング

### Q: "Operation not permitted" エラー
A: `sudo ./led_control` で実行するか、`sudo ./setup_rt_permission.sh` を実行してください

### Q: GPIO制御ができない
A: GPIOグループに所属しているか確認
```bash
groups
sudo usermod -aG gpio $USER
```

### Q: より高い優先度を使いたい
A: `led_control.c` の `RT_PRIORITY` を変更（注意: 99は最高優先度）
```c
#define RT_PRIORITY 90  // 80 → 90
```

---

## 📚 詳細ドキュメント

- `README_RT.md` - PREEMPT_RT機能の詳細説明
- `README.md` - 基本的な使い方

---

作成日: 2025年11月22日
