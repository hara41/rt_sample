#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lgpio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/**
 * PREEMPT_RT対応 リアルタイムLED制御プログラム
 * GPIO17のLEDを高精度タイミングでON/OFFで制御します
 * 実行: sudo ./led_control
 */

#define LED_PIN 17  // GPIO17
#define INTERVAL_NS 1000000  // ナノ秒 (1ms = 1,000,000ns)
#define RT_PRIORITY 80  // リアルタイム優先度 (1-99, 高いほど優先)

static volatile int running = 1;
static int handle = -1;

/**
 * シグナルハンドラー（Ctrl+C対応）
 */
void signal_handler(int signum)
{
    (void)signum;
    running = 0;
}

/**
 * 絶対時刻までスリープ（時刻ドリフトを防ぐ）
 */
static inline void sleep_until(struct timespec *target_time)
{
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, target_time, NULL) != 0) {
        // EINTR の場合は再試行
    }
}

/**
 * timespecに指定ナノ秒を加算
 */
static inline void timespec_add_ns(struct timespec *ts, long long ns)
{
    ts->tv_nsec += ns;
    while (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
}

/**
 * リアルタイムスレッドの設定
 * エラーが発生しても警告を出して続行する
 */
int setup_realtime(void)
{
    struct sched_param param;
    int rt_enabled = 0;
    int memlock_enabled = 0;
    
    // メモリをロックしてページングを防止
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == 0)
    {
        memlock_enabled = 1;
        printf("✓ メモリロック有効\n");
    }
    else
    {
        fprintf(stderr, "⚠ メモリロック失敗: %s\n", strerror(errno));
        fprintf(stderr, "  (sudo権限で実行するとリアルタイム性能が向上します)\n");
    }
    
    // SCHED_FIFOスケジューリングポリシーを設定
    param.sched_priority = RT_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == 0)
    {
        rt_enabled = 1;
        printf("✓ リアルタイムスケジューリング有効 (優先度: %d, SCHED_FIFO)\n", RT_PRIORITY);
    }
    else
    {
        fprintf(stderr, "⚠ リアルタイムスケジューリング失敗: %s\n", strerror(errno));
        fprintf(stderr, "  (sudo権限で実行するとリアルタイム性能が向上します)\n");
    }
    
    if (!rt_enabled && !memlock_enabled)
    {
        printf("\n標準モードで動作します（精度は低下します）\n");
        printf("高精度モードにするには: sudo ./led_control\n");
    }
    else if (rt_enabled && memlock_enabled)
    {
        printf("\n✓ フルリアルタイムモードで動作\n");
    }
    else
    {
        printf("\n部分的なリアルタイムモードで動作\n");
    }
    
    return 0;
}

int main(void)
{
    struct timespec start_time, current_time;
    long long cycle_count = 0;
    long long total_error_ns = 0;
    int led_state = 0;
    
    printf("=== PREEMPT_RT対応 リアルタイムLED制御プログラム ===\n");
    printf("制御周期: %d ns (%.3f ms)\n", INTERVAL_NS, INTERVAL_NS / 1000000.0);
    
    // シグナルハンドラーを設定
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // リアルタイム設定
    setup_realtime();
    
    // GPIOチップを開く
    handle = lgGpiochipOpen(0);
    if (handle < 0)
    {
        fprintf(stderr, "GPIOチップのオープンに失敗しました: %d\n", handle);
        return 1;
    }
    
    // GPIO17を出力モードに設定
    if (lgGpioClaimOutput(handle, 0, LED_PIN, 0) < 0)
    {
        fprintf(stderr, "GPIO17の設定に失敗しました\n");
        lgGpiochipClose(handle);
        return 1;
    }
    
    printf("GPIO17の設定完了\n");
    printf("LED制御を開始します。Ctrl+Cで終了します\n\n");
    
    // 開始時刻を記録し、次のサイクルの目標時刻を設定
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    struct timespec next_time = start_time;
    timespec_add_ns(&next_time, INTERVAL_NS);
    
    // リアルタイム制御ループ（絶対時刻ベース）
    while (running)
    {
        // LED状態を切り替え
        lgGpioWrite(handle, LED_PIN, led_state);
        led_state = !led_state;
        
        cycle_count++;
        
        // 10000サイクルごとに統計を表示
        if (cycle_count % 10000 == 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            long long elapsed_ns = (current_time.tv_sec - start_time.tv_sec) * 1000000000LL +
                                   (current_time.tv_nsec - start_time.tv_nsec);
            long long expected_ns = cycle_count * INTERVAL_NS;
            long long error_ns = elapsed_ns - expected_ns;
            
            printf("サイクル: %lld, 累積誤差: %lld ns (%.3f us), 平均誤差: %.3f ns\n",
                   cycle_count, error_ns, error_ns / 1000.0, 
                   (double)error_ns / cycle_count);
        }
        
        // 次のサイクルの絶対時刻までスリープ（ドリフトなし）
        sleep_until(&next_time);
        
        // 次のサイクルの目標時刻を更新
        timespec_add_ns(&next_time, INTERVAL_NS);
    }
    
    printf("\n終了処理を実行中...\n");
    
    // LED消灯
    lgGpioWrite(handle, LED_PIN, 0);
    
    // クリーンアップ
    lgGpioFree(handle, LED_PIN);
    lgGpiochipClose(handle);
    
    printf("総サイクル数: %lld\n", cycle_count);
    printf("平均誤差: %.3f ns (%.6f us)\n", 
           (double)total_error_ns / cycle_count,
           (double)total_error_ns / cycle_count / 1000.0);
    
    return 0;
}
