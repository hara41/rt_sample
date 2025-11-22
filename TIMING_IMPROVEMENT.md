# タイミング精度改善ガイド

## 🎯 問題: 時刻ドリフト（累積誤差）

### 問題の説明

相対時刻ベースのスリープ（`clock_nanosleep(CLOCK_MONOTONIC, 0, ...)`）を使用すると、以下の理由で誤差が累積します：

1. **処理時間の蓄積**: GPIO操作や統計計算の時間が毎回加算される
2. **スリープの誤差**: カーネルのスケジューリング遅延が累積する
3. **時刻ドリフト**: 時間とともに理想時刻からずれていく

### 修正前の結果

```
サイクル: 10000, 累積誤差: 154,769 us (154 ms), 平均誤差: 15.5 us
サイクル: 20000, 累積誤差: 373,933 us (373 ms), 平均誤差: 26.4 us
サイクル: 70000, 累積誤差: 1,052,995 us (1.05 秒!), 平均誤差: 62.7 us
```

❌ **70,000サイクル（70秒）で1秒以上のズレ**

---

## ✅ 解決策: 絶対時刻ベースのスリープ

### 実装方法

```c
// 絶対時刻を指定してスリープ
clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target_time, NULL);
```

### 主要な変更点

#### 1. スリープ関数の変更

**修正前:**
```c
static inline void precise_sleep(long nanoseconds)
{
    struct timespec req;
    req.tv_sec = nanoseconds / 1000000000L;
    req.tv_nsec = nanoseconds % 1000000000L;
    clock_nanosleep(CLOCK_MONOTONIC, 0, &req, NULL);  // 相対時刻
}
```

**修正後:**
```c
static inline void sleep_until(struct timespec *target_time)
{
    // TIMER_ABSTIME フラグで絶対時刻を指定
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, target_time, NULL);
}

static inline void timespec_add_ns(struct timespec *ts, long long ns)
{
    ts->tv_nsec += ns;
    while (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
}
```

#### 2. メインループの変更

**修正前:**
```c
while (running)
{
    lgGpioWrite(handle, LED_PIN, led_state);
    led_state = !led_state;
    
    // 相対時刻でスリープ → 処理時間分ズレる
    precise_sleep(INTERVAL_NS);
}
```

**修正後:**
```c
// 次のサイクルの目標時刻を計算
struct timespec next_time;
clock_gettime(CLOCK_MONOTONIC, &next_time);
timespec_add_ns(&next_time, INTERVAL_NS);

while (running)
{
    lgGpioWrite(handle, LED_PIN, led_state);
    led_state = !led_state;
    
    // 絶対時刻までスリープ → 処理時間は関係ない
    sleep_until(&next_time);
    
    // 次の目標時刻を更新
    timespec_add_ns(&next_time, INTERVAL_NS);
}
```

---

## 📊 改善結果

### 修正後の精度

```
サイクル: 10000, 累積誤差: -992 us (-0.99 ms), 平均誤差: -99 ns
サイクル: 20000, 累積誤差: -992 us (-0.99 ms), 平均誤差: -49 ns
サイクル: 30000, 累積誤差: -992 us (-0.99 ms), 平均誤差: -33 ns
```

✅ **累積誤差が一定値で安定（ドリフトなし）**

### 精度の比較

| 項目 | 修正前 | 修正後 | 改善率 |
|------|--------|--------|--------|
| 10,000サイクル誤差 | 154.7 ms | 0.99 ms | **156倍** |
| 20,000サイクル誤差 | 373.9 ms | 0.99 ms | **377倍** |
| 70,000サイクル誤差 | 1053 ms | ~1 ms | **1000倍以上** |
| 平均誤差 | 15～63 μs | 33～99 ns | **約500倍** |
| 時刻ドリフト | あり | なし | ∞ |

---

## 💡 理解のポイント

### なぜ絶対時刻ベースが優れているのか？

**相対時刻の場合:**
```
理想時刻:  0ms → 1ms → 2ms → 3ms → 4ms
実際:      0ms → 1.01ms → 2.03ms → 3.06ms → 4.10ms
          処理+遅延  処理+遅延  処理+遅延  処理+遅延
誤差:      0    +0.01   +0.03   +0.06   +0.10  (累積)
```

**絶対時刻の場合:**
```
理想時刻:  0ms → 1ms → 2ms → 3ms → 4ms
実際:      0ms → 1.00ms → 2.00ms → 3.00ms → 4.00ms
          処理  (1msまで待機) (2msまで待機) (3msまで待機)
誤差:      0     0       0       0       0  (累積しない)
```

### 原理

絶対時刻ベースでは、各サイクルで「次に起きるべき絶対時刻」を計算し、その時刻まで正確にスリープします。処理時間がどれだけかかっても、目標時刻までの待機時間が自動的に調整されるため、誤差が累積しません。

---

## 🔧 適用シーン

### 絶対時刻ベースが推奨される場合

✅ 周期的なタスク（LED点滅、PWM、通信プロトコル）
✅ 長時間実行するアプリケーション
✅ 正確なタイミングが必要な制御系
✅ 複数のタスクの同期

### 相対時刻でも問題ない場合

⭕ 単発のスリープ
⭕ タイミング精度が重要でない処理
⭕ バウンス対策などの短時間待機

---

## 📝 まとめ

1. **相対時刻スリープ**: 処理時間が累積し、時刻がドリフトする
2. **絶対時刻スリープ**: 目標時刻に正確に到達し、ドリフトしない
3. **PREEMPT_RT**: どちらの方式でも、リアルタイムカーネルが精度を向上させる
4. **組み合わせ**: 絶対時刻スリープ + PREEMPT_RT = 最高精度

### 改善のインパクト

- **精度**: 500～1000倍向上
- **安定性**: 時刻ドリフトが完全に解消
- **実用性**: 長時間実行でも正確なタイミングを維持

---

作成日: 2025年11月22日
